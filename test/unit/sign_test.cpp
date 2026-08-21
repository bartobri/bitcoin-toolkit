#include "chain/script.hpp"
#include "chain/sign.hpp"
#include "core/address.hpp"
#include "core/hex.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"
#include "core/secp.hpp"
#include "test/unit/check.hpp"

#include <secp256k1.h>
#include <secp256k1_extrakeys.h>
#include <secp256k1_schnorrsig.h>

#include <string>
#include <vector>

namespace {

Privkey secret1() {
    Privkey k;
    k.secret = decode_secret_hex("0000000000000000000000000000000000000000000000000000000000000001");
    k.compressed = true;
    k.network = Network::Main;
    return k;
}

Privkey secret6() {
    Privkey k;
    k.secret = decode_secret_hex("0000000000000000000000000000000000000000000000000000000000000006");
    k.compressed = true;
    k.network = Network::Main;
    return k;
}

Utxo one_utxo(const std::vector<std::uint8_t>& script, std::uint64_t amount) {
    Utxo u;
    const auto display = hex_decode(
        "a882c290a9ce09514de2f57236e349d76b6e4f8437d16d2e132ffc2dada2583c");
    for (std::size_t i = 0; i < 32; ++i) {
        u.txid_internal[i] = display[31 - i];
    }
    u.vout = 0;
    u.amount = amount;
    u.script_pubkey = script;
    u.height = 700000;
    return u;
}

bool verify_ecdsa(const Hash256& msg, const std::vector<std::uint8_t>& der_with_type,
                  const Pubkey& pk) {
    CHECK(der_with_type.size() >= 2);
    CHECK(der_with_type.back() == 0x01);
    secp256k1_ecdsa_signature sig;
    if (secp256k1_ecdsa_signature_parse_der(SecpContext::instance().get(), &sig,
                                            der_with_type.data(), der_with_type.size() - 1) != 1) {
        return false;
    }
    secp256k1_pubkey parsed;
    if (secp256k1_ec_pubkey_parse(SecpContext::instance().get(), &parsed, pk.serialized.data(),
                                  pk.serialized.size()) != 1) {
        return false;
    }
    return secp256k1_ecdsa_verify(SecpContext::instance().get(), &sig, msg.data(), &parsed) == 1;
}

}  // namespace

int main() {
    const Privkey k1 = secret1();
    const Pubkey pk1 = pubkey_from_secret(k1.secret, true, Network::Main);
    const std::string dest = "bc1q7499s50fxu4c0qg23esvm5h8elvqkm33r2tdza";
    const auto dest_script = script_from_address(dest);
    CHECK(dest_script.has_value());

    // P2WPKH of G
    {
        const auto spk = script_from_address("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4");
        CHECK(spk.has_value());
        const std::vector<Utxo> utxos{one_utxo(*spk, 100000)};
        const std::uint64_t est = estimate_sweep_vsize(AddressStyle::P2wpkh, pk1, 1, *dest_script);
        CHECK(est >= 100);
        CHECK(est <= 200);
        const std::uint64_t fee = est * 10;
        Transaction tx = sign_sweep(k1, AddressStyle::P2wpkh, utxos, *dest_script,
                                    static_cast<std::int64_t>(100000 - fee), 800000);
        CHECK(tx.version == 2);
        CHECK(tx.has_witness);
        CHECK(tx.vin.size() == 1);
        CHECK(tx.vout.size() == 1);
        CHECK(tx.vin[0].sequence == 0xfffffffd);
        CHECK(tx.locktime == 800000);
        CHECK(tx.vin[0].witness.size() == 2);
        CHECK(tx.vout[0].script_pubkey == *dest_script);
        CHECK(tx_vsize(tx) <= est);

        const auto code = hex_decode("76a914751e76e8199196d454941c45d1b3a323f1433bd688ac");
        const Hash256 msg = sighash_bip143(tx, 0, code, 100000);
        CHECK(verify_ecdsa(msg, tx.vin[0].witness[0], pk1));
        CHECK(tx.vin[0].witness[1] == pk1.serialized);

        const auto round = parse_tx(serialize_tx(tx, true));
        CHECK(round.vin[0].witness.size() == 2);
        CHECK(txid_display_hex(txid(tx)).size() == 64);
    }

    // P2PKH of G compressed
    {
        const auto spk = script_from_address("1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH");
        CHECK(spk.has_value());
        const std::vector<Utxo> utxos{one_utxo(*spk, 100000)};
        const std::uint64_t est = estimate_sweep_vsize(AddressStyle::P2pkh, pk1, 1, *dest_script);
        const std::uint64_t fee = est * 1;
        Transaction tx = sign_sweep(k1, AddressStyle::P2pkh, utxos, *dest_script,
                                    static_cast<std::int64_t>(100000 - fee), 800000);
        CHECK(!tx.has_witness);
        CHECK(tx.vin[0].script_sig.size() > 70);
        const Hash256 msg = sighash_legacy(tx, 0, *spk);
        // scriptSig is push(sig) push(pubkey)
        CHECK(tx.vin[0].script_sig[0] < 76);
        const std::size_t siglen = tx.vin[0].script_sig[0];
        std::vector<std::uint8_t> sig(tx.vin[0].script_sig.begin() + 1,
                                      tx.vin[0].script_sig.begin() + 1 + siglen);
        CHECK(verify_ecdsa(msg, sig, pk1));
        CHECK(tx_vsize(tx) <= est);
    }

    // P2TR of G (secret 1) and secret 6 (odd-Y)
    for (const Privkey& key : {k1, secret6()}) {
        const Pubkey pk = pubkey_from_secret(key.secret, true, Network::Main);
        const std::string from = encode_p2tr(pk);
        const auto spk = script_from_address(from);
        CHECK(spk.has_value());
        const std::vector<Utxo> utxos{one_utxo(*spk, 100000)};
        const std::uint64_t est = estimate_sweep_vsize(AddressStyle::P2tr, pk, 1, *dest_script);
        const std::uint64_t fee = est * 2;
        Transaction tx = sign_sweep(key, AddressStyle::P2tr, utxos, *dest_script,
                                    static_cast<std::int64_t>(100000 - fee), 800000);
        CHECK(tx.has_witness);
        CHECK(tx.vin[0].witness.size() == 1);
        CHECK(tx.vin[0].witness[0].size() == 64);
        const Hash256 msg = sighash_taproot(tx, 0, utxos);
        const auto output_x = taproot_output_x(pk);
        secp256k1_xonly_pubkey xonly;
        CHECK(secp256k1_xonly_pubkey_parse(SecpContext::instance().get(), &xonly, output_x.data()) ==
              1);
        CHECK(secp256k1_schnorrsig_verify(SecpContext::instance().get(), tx.vin[0].witness[0].data(),
                                          msg.data(), 32, &xonly) == 1);
        CHECK(tx_vsize(tx) <= est);
    }

    // BIP-341 wallet vector 0 internal key
    {
        Privkey k;
        k.secret = decode_secret_hex(
            "6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa");
        k.compressed = true;
        const Pubkey pk = pubkey_from_secret(k.secret, true, Network::Main);
        CHECK(encode_p2tr(pk) ==
              "bc1p2wsldez5mud2yam29q22wgfh9439spgduvct83k3pm50fcxa5dps59h4z5");
        const auto spk = script_from_address(encode_p2tr(pk));
        const std::vector<Utxo> utxos{one_utxo(*spk, 50000)};
        Transaction tx =
            sign_sweep(k, AddressStyle::P2tr, utxos, *dest_script, 49000, 1);
        const Hash256 msg = sighash_taproot(tx, 0, utxos);
        const auto output_x = taproot_output_x(pk);
        secp256k1_xonly_pubkey xonly;
        CHECK(secp256k1_xonly_pubkey_parse(SecpContext::instance().get(), &xonly, output_x.data()) ==
              1);
        CHECK(secp256k1_schnorrsig_verify(SecpContext::instance().get(), tx.vin[0].witness[0].data(),
                                          msg.data(), 32, &xonly) == 1);
    }

    return 0;
}
