#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/hash.hpp"

#ifdef BTK_NO_LEVELDB
constexpr bool kHaveLeveldb = false;
#else
constexpr bool kHaveLeveldb = true;
#endif

enum class IndexState { Missing, Empty, Valid, Junk };

struct OutpointRef {
    Hash256 txid{};
    std::uint32_t vout = 0;
};

struct Credit {
    Hash256 txid{};
    std::uint32_t vout = 0;
    std::string address;
    std::uint64_t amount = 0;
};

// One transaction, in block order. apply() must debit then credit per tx so a
// later tx can spend an output created earlier in the same block.
struct TxEffects {
    std::vector<OutpointRef> spends;
    std::vector<Credit> credits;
};

struct BlockEffects {
    std::uint32_t height = 0;
    Hash256 hash{};
    std::vector<TxEffects> txs;
};

IndexState inspect_index(const std::string& dir);
void destroy_index(const std::string& dir);

class BalanceDb {
public:
    static std::unique_ptr<BalanceDb> open(const std::string& dir, bool writable);

    ~BalanceDb();
    BalanceDb(const BalanceDb&) = delete;
    BalanceDb& operator=(const BalanceDb&) = delete;

    std::uint64_t get_sats(const std::string& addr) const;
    bool get_outpoint(const Hash256& txid, std::uint32_t vout, std::string& addr,
                      std::uint64_t& amount) const;
    bool has_metadata() const;
    std::uint32_t height() const;
    Hash256 tip() const;

    void apply(const BlockEffects& fx);

private:
    BalanceDb();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

std::string default_balance_dir();
void ensure_btk_home();
