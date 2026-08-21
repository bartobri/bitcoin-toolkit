#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "chain/transaction.hpp"
#include "core/address.hpp"
#include "core/hash.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"

struct Utxo {
    std::array<std::uint8_t, 32> txid_internal{};
    std::uint32_t vout = 0;
    std::uint64_t amount = 0;
    std::vector<std::uint8_t> script_pubkey;
    bool coinbase = false;
    std::uint32_t height = 0;
};

std::uint64_t tx_vsize(const Transaction& tx);

// Conservative vsize of a 1-output sweep (73-byte dummy ECDSA, 64-byte Schnorr).
std::uint64_t estimate_sweep_vsize(AddressStyle style, const Pubkey& pk, std::size_t n_inputs,
                                   const std::vector<std::uint8_t>& dest_script);

Transaction sign_sweep(const Privkey& key, AddressStyle style, const std::vector<Utxo>& utxos,
                       const std::vector<std::uint8_t>& dest_script, std::int64_t output_sats,
                       std::uint32_t locktime);

std::string txid_display_hex(const Hash256& internal);

Hash256 sighash_legacy(const Transaction& tx, std::size_t index,
                       const std::vector<std::uint8_t>& script_code);
Hash256 sighash_bip143(const Transaction& tx, std::size_t index,
                       const std::vector<std::uint8_t>& script_code, std::int64_t amount);
Hash256 sighash_taproot(const Transaction& tx, std::size_t index, const std::vector<Utxo>& utxos);
