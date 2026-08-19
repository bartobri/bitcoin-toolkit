#include "chain/inflow_db.hpp"

#include "util/error.hpp"

#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <utility>

#ifndef BTK_NO_LEVELDB
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#endif

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw BtkError("inflow", message);
}

void put_le32(std::string& s, std::uint32_t v) {
    s.push_back(static_cast<char>(v));
    s.push_back(static_cast<char>(v >> 8));
    s.push_back(static_cast<char>(v >> 16));
    s.push_back(static_cast<char>(v >> 24));
}

void put_le64(std::string& s, std::uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        s.push_back(static_cast<char>(v >> (8 * i)));
    }
}

std::uint32_t get_le32(const std::string& s, std::size_t off = 0) {
    return static_cast<std::uint32_t>(static_cast<unsigned char>(s[off])) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 1])) << 8) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 2])) << 16) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 3])) << 24);
}

std::uint64_t get_le64(const std::string& s, std::size_t off = 0) {
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v |= static_cast<std::uint64_t>(static_cast<unsigned char>(s[off + static_cast<std::size_t>(i)]))
             << (8 * i);
    }
    return v;
}

std::string key_a(const std::string& addr) {
    return std::string("A") + addr;
}

std::string encode_row(const InflowRow& row) {
    std::string s;
    put_le64(s, row.sats);
    put_le64(s, row.count);
    put_le32(s, row.last);
    return s;
}

bool decode_row(const std::string& raw, InflowRow& row) {
    if (raw.size() != 20) {
        return false;
    }
    row.sats = get_le64(raw, 0);
    row.count = get_le64(raw, 8);
    row.last = get_le32(raw, 16);
    return true;
}

#ifndef BTK_NO_LEVELDB

bool read_key(leveldb::DB* db, const std::string& key, std::string& value) {
    const leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
    if (s.IsNotFound()) {
        return false;
    }
    if (!s.ok()) {
        fail("leveldb read failed");
    }
    return true;
}

#endif

}  // namespace

std::string default_inflow_dir() {
    const char* home = std::getenv("HOME");
    if (home == nullptr || home[0] == '\0') {
        fail("HOME is not set");
    }
    return std::string(home) + "/.btk/inflow";
}

struct InflowDb::Impl {
#ifndef BTK_NO_LEVELDB
    std::unique_ptr<leveldb::DB> db;
    bool writable = false;
#endif
};

InflowDb::InflowDb() : impl_(std::make_unique<Impl>()) {}

InflowDb::~InflowDb() = default;

std::unique_ptr<InflowDb> InflowDb::open(const std::string& dir, bool writable) {
#ifndef BTK_NO_LEVELDB
    auto out = std::unique_ptr<InflowDb>(new InflowDb());
    leveldb::Options opt;
    opt.create_if_missing = writable;
    leveldb::DB* raw = nullptr;
    const leveldb::Status s = leveldb::DB::Open(opt, dir, &raw);
    if (!s.ok() || raw == nullptr) {
        fail(writable ? "cannot create inflow database"
                      : "inflow database not found (run btk inflow --sync)");
    }
    out->impl_->db.reset(raw);
    out->impl_->writable = writable;
    return out;
#else
    (void)dir;
    (void)writable;
    fail("this build was compiled without LevelDB (install libleveldb-dev and rebuild)");
#endif
}

InflowRow InflowDb::get_row(const std::string& addr) const {
#ifndef BTK_NO_LEVELDB
    std::string raw;
    InflowRow row;
    if (!read_key(impl_->db.get(), key_a(addr), raw) || !decode_row(raw, row)) {
        return {};
    }
    return row;
#else
    (void)addr;
    return {};
#endif
}

bool InflowDb::has_metadata() const {
#ifndef BTK_NO_LEVELDB
    std::string h;
    std::string t;
    return read_key(impl_->db.get(), "Mheight", h) && h.size() == 4 &&
           read_key(impl_->db.get(), "Mtip", t) && t.size() == 32;
#else
    return false;
#endif
}

std::uint32_t InflowDb::height() const {
#ifndef BTK_NO_LEVELDB
    std::string h;
    if (!read_key(impl_->db.get(), "Mheight", h) || h.size() != 4) {
        fail("inflow database not found (run btk inflow --sync)");
    }
    return get_le32(h);
#else
    return 0;
#endif
}

Hash256 InflowDb::tip() const {
#ifndef BTK_NO_LEVELDB
    std::string t;
    if (!read_key(impl_->db.get(), "Mtip", t) || t.size() != 32) {
        fail("inflow database not found (run btk inflow --sync)");
    }
    Hash256 out{};
    std::memcpy(out.data(), t.data(), 32);
    return out;
#else
    return {};
#endif
}

void InflowDb::apply(const BlockEffects& fx) {
#ifndef BTK_NO_LEVELDB
    if (!impl_->writable) {
        fail("internal: read-only database");
    }
    leveldb::WriteBatch batch;
    std::unordered_map<std::string, InflowRow> rows;

    auto current = [&](const std::string& addr) -> InflowRow {
        auto it = rows.find(addr);
        if (it != rows.end()) {
            return it->second;
        }
        const InflowRow v = get_row(addr);
        rows.emplace(addr, v);
        return v;
    };

    for (const TxEffects& te : fx.txs) {
        std::unordered_map<std::string, std::uint64_t> by_addr;
        for (const Credit& c : te.credits) {
            if (c.address.empty() || c.amount == 0) {
                continue;
            }
            by_addr[c.address] += c.amount;
        }
        for (const auto& kv : by_addr) {
            InflowRow row = current(kv.first);
            row.sats += kv.second;
            row.count += 1;
            if (fx.time > row.last) {
                row.last = fx.time;
            }
            rows[kv.first] = row;
        }
    }

    for (const auto& kv : rows) {
        batch.Put(key_a(kv.first), encode_row(kv.second));
    }

    std::string hv;
    put_le32(hv, fx.height);
    batch.Put("Mheight", hv);
    std::string tv(reinterpret_cast<const char*>(fx.hash.data()), 32);
    batch.Put("Mtip", tv);

    const leveldb::Status s = impl_->db->Write(leveldb::WriteOptions(), &batch);
    if (!s.ok()) {
        fail("leveldb write failed");
    }
#else
    (void)fx;
    fail("this build was compiled without LevelDB (install libleveldb-dev and rebuild)");
#endif
}
