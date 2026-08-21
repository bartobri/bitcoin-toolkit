#include "chain/sign.hpp"

#include "chain/compactsize.hpp"
#include "core/hash.hpp"
#include "core/hex.hpp"
#include "core/secp.hpp"
#include "util/error.hpp"

#include <algorithm>
#include <cstring>
#include <secp256k1.h>
#include <secp256k1_extrakeys.h>
#include <secp256k1_schnorrsig.h>

namespace {

[[noreturn]] void fail_sign(const std::string& message) {
    throw BtkError("", message);
}

void write_le32(std::vector<std::uint8_t>& o, std::uint32_t v) {
    o.push_back(static_cast<std::uint8_t>(v));
    o.push_back(static_cast<std::uint8_t>(v >> 8));
    o.push_back(static_cast<std::uint8_t>(v >> 16));
    o.push_back(static_cast<std::uint8_t>(v >> 24));
}

void write_le64(std::vector<std::uint8_t>& o, std::uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        o.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
    }
}

void write_script(std::vector<std::uint8_t>& o, const std::vector<std::uint8_t>& script) {
    write_compact_size(o, script.size());
    o.insert(o.end(), script.begin(), script.end());
}

std::vector<std::uint8_t> push_data(const std::vector<std::uint8_t>& data) {
    if (data.size() > 75) {
        fail_sign("could not sign");
    }
    std::vector<std::uint8_t> o;
    o.push_back(static_cast<std::uint8_t>(data.size()));
    o.insert(o.end(), data.begin(), data.end());
    return o;
}

constexpr std::uint32_t kSequenceRbf = 0xfffffffd;
constexpr std::uint32_t kSighashAll = 1;
constexpr std::size_t kDummyEcdsa = 73;
constexpr std::size_t kSchnorr = 64;

bool is_witness_style(AddressStyle style) {
    return style != AddressStyle::P2pkh;
}

Transaction make_skeleton(const std::vector<Utxo>& utxos, const std::vector<std::uint8_t>& dest,
                          std::int64_t output_sats, std::uint32_t locktime, bool witness) {
    Transaction tx;
    tx.version = 2;
    tx.has_witness = witness;
    tx.locktime = locktime;
    tx.vin.reserve(utxos.size());
    for (const Utxo& u : utxos) {
        TxIn in;
        in.prev_txid = u.txid_internal;
        in.prev_vout = u.vout;
        in.sequence = kSequenceRbf;
        tx.vin.push_back(std::move(in));
    }
    TxOut out;
    out.value = output_sats;
    out.script_pubkey = dest;
    tx.vout.push_back(std::move(out));
    return tx;
}

void fill_dummy(Transaction& tx, AddressStyle style, const Pubkey& pk) {
    std::vector<std::uint8_t> dummy_sig(kDummyEcdsa, 0x00);
    dummy_sig[0] = 0x30;
    for (TxIn& in : tx.vin) {
        if (style == AddressStyle::P2pkh) {
            auto a = push_data(dummy_sig);
            auto b = push_data(pk.serialized);
            in.script_sig = a;
            in.script_sig.insert(in.script_sig.end(), b.begin(), b.end());
            in.witness.clear();
        } else if (style == AddressStyle::P2wpkh) {
            in.script_sig.clear();
            in.witness = {dummy_sig, pk.serialized};
        } else {
            in.script_sig.clear();
            in.witness = {std::vector<std::uint8_t>(kSchnorr, 0x00)};
        }
    }
}

std::vector<std::uint8_t> p2wpkh_script_code(const std::vector<std::uint8_t>& spk) {
    if (spk.size() != 22 || spk[0] != 0x00 || spk[1] != 20) {
        fail_sign("unexpected scriptPubKey");
    }
    std::vector<std::uint8_t> code;
    code.reserve(25);
    code.push_back(0x76);
    code.push_back(0xa9);
    code.push_back(20);
    code.insert(code.end(), spk.begin() + 2, spk.end());
    code.push_back(0x88);
    code.push_back(0xac);
    return code;
}

std::vector<std::uint8_t> ecdsa_der(const Hash256& msg, const Secret& secret) {
    secp256k1_ecdsa_signature sig;
    if (secp256k1_ecdsa_sign(SecpContext::instance().get(), &sig, msg.data(), secret.data(),
                             nullptr, nullptr) != 1) {
        fail_sign("could not sign");
    }
    unsigned char der[80];
    std::size_t derlen = sizeof(der);
    if (secp256k1_ecdsa_signature_serialize_der(SecpContext::instance().get(), der, &derlen, &sig) !=
        1) {
        fail_sign("could not sign");
    }
    std::vector<std::uint8_t> out(der, der + derlen);
    out.push_back(0x01);
    return out;
}

std::vector<std::uint8_t> schnorr_sig(const Hash256& msg, const Secret& secret) {
    const Pubkey pk = pubkey_from_secret(secret, true, Network::Main);
    if (pk.serialized.size() != 33) {
        fail_sign("uncompressed key cannot produce p2wpkh or p2tr");
    }
    const Hash256 tweak = tagged_hash("TapTweak", pk.serialized.data() + 1, 32);

    secp256k1_context* ctx = SecpContext::instance().get();
    secp256k1_keypair keypair;
    if (secp256k1_keypair_create(ctx, &keypair, secret.data()) != 1) {
        fail_sign("could not sign");
    }
    const bool tweak_zero = std::all_of(tweak.begin(), tweak.end(), [](std::uint8_t b) {
        return b == 0;
    });
    if (!tweak_zero && secp256k1_ec_seckey_verify(ctx, tweak.data()) != 1) {
        fail_sign("taproot tweak out of range");
    }
    if (secp256k1_keypair_xonly_tweak_add(ctx, &keypair, tweak.data()) != 1) {
        fail_sign("taproot tweak out of range");
    }
    unsigned char sig[64];
    unsigned char aux[32] = {};
    if (secp256k1_schnorrsig_sign32(ctx, sig, msg.data(), &keypair, aux) != 1) {
        fail_sign("could not sign");
    }
    return {sig, sig + 64};
}

}  // namespace

std::uint64_t tx_vsize(const Transaction& tx) {
    const auto base = serialize_tx(tx, false);
    if (!tx.has_witness) {
        return base.size();
    }
    const auto full = serialize_tx(tx, true);
    const std::uint64_t weight =
        static_cast<std::uint64_t>(base.size()) * 3 + static_cast<std::uint64_t>(full.size());
    return (weight + 3) / 4;
}

std::uint64_t estimate_sweep_vsize(AddressStyle style, const Pubkey& pk, std::size_t n_inputs,
                                   const std::vector<std::uint8_t>& dest_script) {
    if (n_inputs == 0) {
        return 0;
    }
    std::vector<Utxo> dummy(n_inputs);
    Transaction tx = make_skeleton(dummy, dest_script, 0, 0, is_witness_style(style));
    fill_dummy(tx, style, pk);
    return tx_vsize(tx);
}

std::string txid_display_hex(const Hash256& internal) {
    Hash256 display{};
    for (std::size_t i = 0; i < 32; ++i) {
        display[i] = internal[31 - i];
    }
    return hex_encode(display);
}

Hash256 sighash_legacy(const Transaction& tx, std::size_t index,
                       const std::vector<std::uint8_t>& script_code) {
    if (index >= tx.vin.size()) {
        fail_sign("could not sign");
    }
    std::vector<std::uint8_t> pre;
    write_le32(pre, static_cast<std::uint32_t>(tx.version));
    write_compact_size(pre, tx.vin.size());
    for (std::size_t i = 0; i < tx.vin.size(); ++i) {
        const TxIn& in = tx.vin[i];
        pre.insert(pre.end(), in.prev_txid.begin(), in.prev_txid.end());
        write_le32(pre, in.prev_vout);
        if (i == index) {
            write_script(pre, script_code);
        } else {
            write_compact_size(pre, 0);
        }
        write_le32(pre, in.sequence);
    }
    write_compact_size(pre, tx.vout.size());
    for (const TxOut& out : tx.vout) {
        write_le64(pre, static_cast<std::uint64_t>(out.value));
        write_script(pre, out.script_pubkey);
    }
    write_le32(pre, tx.locktime);
    write_le32(pre, kSighashAll);
    return hash256(pre);
}

Hash256 sighash_bip143(const Transaction& tx, std::size_t index,
                       const std::vector<std::uint8_t>& script_code, std::int64_t amount) {
    if (index >= tx.vin.size()) {
        fail_sign("could not sign");
    }

    std::vector<std::uint8_t> prevouts;
    std::vector<std::uint8_t> sequences;
    for (const TxIn& in : tx.vin) {
        prevouts.insert(prevouts.end(), in.prev_txid.begin(), in.prev_txid.end());
        write_le32(prevouts, in.prev_vout);
        write_le32(sequences, in.sequence);
    }
    std::vector<std::uint8_t> outputs;
    for (const TxOut& out : tx.vout) {
        write_le64(outputs, static_cast<std::uint64_t>(out.value));
        write_script(outputs, out.script_pubkey);
    }

    const Hash256 hash_prevouts = hash256(prevouts);
    const Hash256 hash_sequence = hash256(sequences);
    const Hash256 hash_outputs = hash256(outputs);

    std::vector<std::uint8_t> pre;
    write_le32(pre, static_cast<std::uint32_t>(tx.version));
    pre.insert(pre.end(), hash_prevouts.begin(), hash_prevouts.end());
    pre.insert(pre.end(), hash_sequence.begin(), hash_sequence.end());
    const TxIn& in = tx.vin[index];
    pre.insert(pre.end(), in.prev_txid.begin(), in.prev_txid.end());
    write_le32(pre, in.prev_vout);
    write_script(pre, script_code);
    write_le64(pre, static_cast<std::uint64_t>(amount));
    write_le32(pre, in.sequence);
    pre.insert(pre.end(), hash_outputs.begin(), hash_outputs.end());
    write_le32(pre, tx.locktime);
    write_le32(pre, kSighashAll);
    return hash256(pre);
}

Hash256 sighash_taproot(const Transaction& tx, std::size_t index, const std::vector<Utxo>& utxos) {
    if (index >= tx.vin.size() || utxos.size() != tx.vin.size()) {
        fail_sign("could not sign");
    }

    std::vector<std::uint8_t> prevouts;
    std::vector<std::uint8_t> amounts;
    std::vector<std::uint8_t> scripts;
    std::vector<std::uint8_t> sequences;
    for (std::size_t i = 0; i < tx.vin.size(); ++i) {
        const TxIn& in = tx.vin[i];
        prevouts.insert(prevouts.end(), in.prev_txid.begin(), in.prev_txid.end());
        write_le32(prevouts, in.prev_vout);
        write_le64(amounts, utxos[i].amount);
        write_script(scripts, utxos[i].script_pubkey);
        write_le32(sequences, in.sequence);
    }
    std::vector<std::uint8_t> outputs;
    for (const TxOut& out : tx.vout) {
        write_le64(outputs, static_cast<std::uint64_t>(out.value));
        write_script(outputs, out.script_pubkey);
    }

    const Hash256 sha_prevouts = sha256(prevouts);
    const Hash256 sha_amounts = sha256(amounts);
    const Hash256 sha_scripts = sha256(scripts);
    const Hash256 sha_sequences = sha256(sequences);
    const Hash256 sha_outputs = sha256(outputs);

    std::vector<std::uint8_t> data;
    data.push_back(0x00);  // epoch
    data.push_back(0x00);  // SIGHASH_DEFAULT
    write_le32(data, static_cast<std::uint32_t>(tx.version));
    write_le32(data, tx.locktime);
    data.insert(data.end(), sha_prevouts.begin(), sha_prevouts.end());
    data.insert(data.end(), sha_amounts.begin(), sha_amounts.end());
    data.insert(data.end(), sha_scripts.begin(), sha_scripts.end());
    data.insert(data.end(), sha_sequences.begin(), sha_sequences.end());
    data.insert(data.end(), sha_outputs.begin(), sha_outputs.end());
    data.push_back(0x00);  // spend_type: key-path, no annex
    write_le32(data, static_cast<std::uint32_t>(index));
    return tagged_hash("TapSighash", data.data(), data.size());
}

Transaction sign_sweep(const Privkey& key, AddressStyle style, const std::vector<Utxo>& utxos,
                       const std::vector<std::uint8_t>& dest_script, std::int64_t output_sats,
                       std::uint32_t locktime) {
    if (utxos.empty()) {
        fail_sign("no unspent outputs");
    }
    const Pubkey pk = pubkey_from_secret(key.secret, key.compressed, key.network);
    Transaction tx = make_skeleton(utxos, dest_script, output_sats, locktime, is_witness_style(style));

    for (std::size_t i = 0; i < tx.vin.size(); ++i) {
        TxIn& in = tx.vin[i];
        if (style == AddressStyle::P2pkh) {
            const Hash256 msg = sighash_legacy(tx, i, utxos[i].script_pubkey);
            const auto sig = ecdsa_der(msg, key.secret);
            auto a = push_data(sig);
            auto b = push_data(pk.serialized);
            in.script_sig = a;
            in.script_sig.insert(in.script_sig.end(), b.begin(), b.end());
        } else if (style == AddressStyle::P2wpkh) {
            const auto code = p2wpkh_script_code(utxos[i].script_pubkey);
            const Hash256 msg =
                sighash_bip143(tx, i, code, static_cast<std::int64_t>(utxos[i].amount));
            const auto sig = ecdsa_der(msg, key.secret);
            in.script_sig.clear();
            in.witness = {sig, pk.serialized};
        } else {
            const Hash256 msg = sighash_taproot(tx, i, utxos);
            in.script_sig.clear();
            in.witness = {schnorr_sig(msg, key.secret)};
        }
    }
    return tx;
}
