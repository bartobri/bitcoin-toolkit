#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "chain/balance_db.hpp"

struct InflowRow {
    std::uint64_t sats = 0;
    std::uint64_t count = 0;
    std::uint32_t last = 0;
};

class InflowDb {
public:
    static std::unique_ptr<InflowDb> open(const std::string& dir, bool writable);

    ~InflowDb();
    InflowDb(const InflowDb&) = delete;
    InflowDb& operator=(const InflowDb&) = delete;

    InflowRow get_row(const std::string& addr) const;
    bool has_metadata() const;
    std::uint32_t height() const;
    Hash256 tip() const;

    void apply(const BlockEffects& fx);

private:
    InflowDb();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

std::string default_inflow_dir();
