#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/hash.hpp"

struct TxIn {
    std::array<std::uint8_t, 32> prev_txid{};
    std::uint32_t prev_vout = 0;
    std::vector<std::uint8_t> script_sig;
    std::uint32_t sequence = 0;
    std::vector<std::vector<std::uint8_t>> witness;
};

struct TxOut {
    std::int64_t value = 0;
    std::vector<std::uint8_t> script_pubkey;
};

struct Transaction {
    std::int32_t version = 0;
    bool has_witness = false;
    std::vector<TxIn> vin;
    std::vector<TxOut> vout;
    std::uint32_t locktime = 0;
};

struct Block {
    std::int32_t version = 0;
    std::array<std::uint8_t, 32> prev{};
    std::array<std::uint8_t, 32> merkle{};
    std::uint32_t time = 0;
    std::uint32_t bits = 0;
    std::uint32_t nonce = 0;
    std::vector<Transaction> txs;
};

bool is_null_prevout(const TxIn& in);

Transaction parse_tx(const std::uint8_t* data, std::size_t len, std::size_t* consumed = nullptr);
Transaction parse_tx(const std::vector<std::uint8_t>& data);

std::vector<std::uint8_t> serialize_tx(const Transaction& tx, bool include_witness);

Hash256 txid(const Transaction& tx);
Hash256 wtxid(const Transaction& tx);

Block parse_block(const std::uint8_t* data, std::size_t len);
Block parse_block(const std::vector<std::uint8_t>& data);

Hash256 block_hash(const Block& block);
std::vector<std::uint8_t> serialize_block_header(const Block& block);
