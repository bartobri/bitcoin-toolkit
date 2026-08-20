#include "chain/transaction.hpp"

#include "chain/compactsize.hpp"
#include "util/error.hpp"

#include <algorithm>
#include <cstring>

namespace {

[[noreturn]] void fail_tx() {
    throw BtkError("balance", "invalid transaction");
}

[[noreturn]] void fail_block() {
    throw BtkError("balance", "invalid block");
}

std::uint32_t read_le32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}

std::uint64_t read_le64(const std::uint8_t* p) {
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v |= static_cast<std::uint64_t>(p[i]) << (8 * i);
    }
    return v;
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

struct Cursor {
    const std::uint8_t* data;
    std::size_t len;
    std::size_t off = 0;

    void need(std::size_t n) const {
        if (off + n > len) {
            fail_tx();
        }
    }

    std::uint8_t u8() {
        need(1);
        return data[off++];
    }

    std::uint32_t le32() {
        need(4);
        const std::uint32_t v = read_le32(data + off);
        off += 4;
        return v;
    }

    std::uint64_t le64() {
        need(8);
        const std::uint64_t v = read_le64(data + off);
        off += 8;
        return v;
    }

    std::uint64_t compact() {
        return read_compact_size(data, len, off);
    }

    std::vector<std::uint8_t> bytes(std::size_t n) {
        need(n);
        std::vector<std::uint8_t> out(data + off, data + off + n);
        off += n;
        return out;
    }

    void copy32(std::array<std::uint8_t, 32>& dest) {
        need(32);
        std::memcpy(dest.data(), data + off, 32);
        off += 32;
    }

    std::size_t remain() const { return off <= len ? len - off : 0; }

    // Each counted item occupies at least one byte. Do not cap at a small
    // constant: mainnet block 761249 has a tx with 500003 witness items.
    void bound_count(std::uint64_t n) const {
        if (n > remain()) {
            fail_tx();
        }
    }
};

void write_vin_vout(std::vector<std::uint8_t>& o, const Transaction& tx) {
    write_compact_size(o, tx.vin.size());
    for (const TxIn& in : tx.vin) {
        o.insert(o.end(), in.prev_txid.begin(), in.prev_txid.end());
        write_le32(o, in.prev_vout);
        write_compact_size(o, in.script_sig.size());
        o.insert(o.end(), in.script_sig.begin(), in.script_sig.end());
        write_le32(o, in.sequence);
    }
    write_compact_size(o, tx.vout.size());
    for (const TxOut& out : tx.vout) {
        write_le64(o, static_cast<std::uint64_t>(out.value));
        write_compact_size(o, out.script_pubkey.size());
        o.insert(o.end(), out.script_pubkey.begin(), out.script_pubkey.end());
    }
}

}  // namespace

bool is_null_prevout(const TxIn& in) {
    return std::all_of(in.prev_txid.begin(), in.prev_txid.end(), [](std::uint8_t b) {
        return b == 0;
    });
}

Transaction parse_tx(const std::uint8_t* data, std::size_t len, std::size_t* consumed) {
    if (data == nullptr || len < 10) {
        fail_tx();
    }
    Cursor c{data, len, 0};
    Transaction tx;
    tx.version = static_cast<std::int32_t>(c.le32());

    c.need(1);
    if (data[c.off] == 0x00) {
        c.u8();
        const std::uint8_t flag = c.u8();
        if (flag == 0) {
            fail_tx();
        }
        tx.has_witness = (flag & 1u) != 0;
        if (!tx.has_witness) {
            fail_tx();
        }
    }

    const std::uint64_t n_in = c.compact();
    c.bound_count(n_in);
    tx.vin.resize(static_cast<std::size_t>(n_in));
    for (TxIn& in : tx.vin) {
        c.copy32(in.prev_txid);
        in.prev_vout = c.le32();
        const std::uint64_t slen = c.compact();
        if (slen > len) {
            fail_tx();
        }
        in.script_sig = c.bytes(static_cast<std::size_t>(slen));
        in.sequence = c.le32();
    }

    const std::uint64_t n_out = c.compact();
    c.bound_count(n_out);
    tx.vout.resize(static_cast<std::size_t>(n_out));
    for (TxOut& out : tx.vout) {
        out.value = static_cast<std::int64_t>(c.le64());
        const std::uint64_t slen = c.compact();
        if (slen > len) {
            fail_tx();
        }
        out.script_pubkey = c.bytes(static_cast<std::size_t>(slen));
    }

    if (tx.has_witness) {
        for (TxIn& in : tx.vin) {
            const std::uint64_t n_items = c.compact();
            c.bound_count(n_items);
            in.witness.resize(static_cast<std::size_t>(n_items));
            for (auto& item : in.witness) {
                const std::uint64_t ilen = c.compact();
                if (ilen > len) {
                    fail_tx();
                }
                item = c.bytes(static_cast<std::size_t>(ilen));
            }
        }
    }

    tx.locktime = c.le32();
    if (consumed != nullptr) {
        *consumed = c.off;
    }
    return tx;
}

Transaction parse_tx(const std::vector<std::uint8_t>& data) {
    std::size_t used = 0;
    Transaction tx = parse_tx(data.data(), data.size(), &used);
    if (used != data.size()) {
        fail_tx();
    }
    return tx;
}

std::vector<std::uint8_t> serialize_tx(const Transaction& tx, bool include_witness) {
    const bool wit = include_witness && tx.has_witness;
    std::vector<std::uint8_t> o;
    write_le32(o, static_cast<std::uint32_t>(tx.version));
    if (wit) {
        o.push_back(0x00);
        o.push_back(0x01);
    }
    write_vin_vout(o, tx);
    if (wit) {
        for (const TxIn& in : tx.vin) {
            write_compact_size(o, in.witness.size());
            for (const auto& item : in.witness) {
                write_compact_size(o, item.size());
                o.insert(o.end(), item.begin(), item.end());
            }
        }
    }
    write_le32(o, tx.locktime);
    return o;
}

Hash256 txid(const Transaction& tx) {
    const auto bytes = serialize_tx(tx, false);
    return hash256(bytes);
}

Hash256 wtxid(const Transaction& tx) {
    const auto bytes = serialize_tx(tx, true);
    return hash256(bytes);
}

std::vector<std::uint8_t> serialize_block_header(const Block& block) {
    std::vector<std::uint8_t> o;
    o.reserve(80);
    write_le32(o, static_cast<std::uint32_t>(block.version));
    o.insert(o.end(), block.prev.begin(), block.prev.end());
    o.insert(o.end(), block.merkle.begin(), block.merkle.end());
    write_le32(o, block.time);
    write_le32(o, block.bits);
    write_le32(o, block.nonce);
    return o;
}

Hash256 block_hash(const Block& block) {
    const auto hdr = serialize_block_header(block);
    return hash256(hdr);
}

Block parse_block(const std::uint8_t* data, std::size_t len) {
    if (data == nullptr || len < 81) {
        fail_block();
    }
    Block b;
    b.version = static_cast<std::int32_t>(read_le32(data));
    std::memcpy(b.prev.data(), data + 4, 32);
    std::memcpy(b.merkle.data(), data + 36, 32);
    b.time = read_le32(data + 68);
    b.bits = read_le32(data + 72);
    b.nonce = read_le32(data + 76);

    std::size_t off = 80;
    const std::uint64_t n_tx = read_compact_size(data, len, off);
    if (off > len || n_tx > len - off) {
        fail_block();
    }
    b.txs.reserve(static_cast<std::size_t>(n_tx));
    for (std::uint64_t i = 0; i < n_tx; ++i) {
        std::size_t used = 0;
        try {
            b.txs.push_back(parse_tx(data + off, len - off, &used));
        } catch (const BtkError&) {
            fail_block();
        }
        if (used == 0) {
            fail_block();
        }
        off += used;
    }
    if (off != len) {
        fail_block();
    }
    return b;
}

Block parse_block(const std::vector<std::uint8_t>& data) {
    return parse_block(data.data(), data.size());
}
