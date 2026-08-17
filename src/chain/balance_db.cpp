#include "chain/balance_db.hpp"

#include "chain/compactsize.hpp"
#include "util/error.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
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
    throw BtkError("balance", message);
}

bool path_exists(const std::string& path, bool& is_dir) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    is_dir = S_ISDIR(st.st_mode);
    return true;
}

bool directory_empty(const std::string& path) {
    DIR* d = opendir(path.c_str());
    if (d == nullptr) {
        return false;
    }
    bool empty = true;
    while (dirent* e = readdir(d)) {
        if (std::strcmp(e->d_name, ".") != 0 && std::strcmp(e->d_name, "..") != 0) {
            empty = false;
            break;
        }
    }
    closedir(d);
    return empty;
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

std::string key_o(const Hash256& txid, std::uint32_t vout) {
    std::string k;
    k.push_back('O');
    k.append(reinterpret_cast<const char*>(txid.data()), 32);
    put_le32(k, vout);
    return k;
}

std::string encode_out_value(const std::string& addr, std::uint64_t amount) {
    std::vector<std::uint8_t> buf;
    write_compact_size(buf, addr.size());
    buf.insert(buf.end(), addr.begin(), addr.end());
    std::string s(reinterpret_cast<const char*>(buf.data()), buf.size());
    put_le64(s, amount);
    return s;
}

bool decode_out_value(const std::string& raw, std::string& addr, std::uint64_t& amount) {
    if (raw.size() < 9) {
        return false;
    }
    const auto* data = reinterpret_cast<const std::uint8_t*>(raw.data());
    std::size_t off = 0;
    try {
        const std::uint64_t n = read_compact_size(data, raw.size(), off);
        if (off + n + 8 != raw.size()) {
            return false;
        }
        addr.assign(raw, off, static_cast<std::size_t>(n));
        amount = get_le64(raw, off + static_cast<std::size_t>(n));
        return true;
    } catch (const BtkError&) {
        return false;
    }
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

std::string default_balance_dir() {
    const char* home = std::getenv("HOME");
    if (home == nullptr || home[0] == '\0') {
        fail("HOME is not set");
    }
    return std::string(home) + "/.btk/balance";
}

void ensure_btk_home() {
    const char* home = std::getenv("HOME");
    if (home == nullptr || home[0] == '\0') {
        fail("HOME is not set");
    }
    const std::string dir = std::string(home) + "/.btk";
    if (mkdir(dir.c_str(), 0700) != 0 && errno != EEXIST) {
        fail("cannot create ~/.btk");
    }
}

IndexState inspect_index(const std::string& dir) {
    bool is_dir = false;
    if (!path_exists(dir, is_dir)) {
        return IndexState::Missing;
    }
    if (!is_dir) {
        return IndexState::Junk;
    }
    if (directory_empty(dir)) {
        return IndexState::Empty;
    }
#ifndef BTK_NO_LEVELDB
    leveldb::Options opt;
    opt.create_if_missing = false;
    leveldb::DB* raw = nullptr;
    const leveldb::Status s = leveldb::DB::Open(opt, dir, &raw);
    if (!s.ok() || raw == nullptr) {
        return IndexState::Junk;
    }
    std::unique_ptr<leveldb::DB> db(raw);
    std::string h;
    std::string t;
    if (read_key(db.get(), "Mheight", h) && h.size() == 4 && read_key(db.get(), "Mtip", t) &&
        t.size() == 32) {
        return IndexState::Valid;
    }
    return IndexState::Junk;
#else
    (void)dir;
    return IndexState::Junk;
#endif
}

void destroy_index(const std::string& dir) {
#ifndef BTK_NO_LEVELDB
    const leveldb::Status s = leveldb::DestroyDB(dir, leveldb::Options());
    if (!s.ok()) {
        fail("cannot remove balance database");
    }
#endif
    bool is_dir = false;
    if (!path_exists(dir, is_dir) || !is_dir) {
        return;
    }
    DIR* d = opendir(dir.c_str());
    if (d == nullptr) {
        return;
    }
    while (dirent* e = readdir(d)) {
        if (std::strcmp(e->d_name, ".") == 0 || std::strcmp(e->d_name, "..") == 0) {
            continue;
        }
        const std::string p = dir + "/" + e->d_name;
        unlink(p.c_str());
    }
    closedir(d);
    rmdir(dir.c_str());
}

struct BalanceDb::Impl {
#ifndef BTK_NO_LEVELDB
    std::unique_ptr<leveldb::DB> db;
    bool writable = false;
#endif
};

BalanceDb::BalanceDb() : impl_(std::make_unique<Impl>()) {}

BalanceDb::~BalanceDb() = default;

std::unique_ptr<BalanceDb> BalanceDb::open(const std::string& dir, bool writable) {
#ifndef BTK_NO_LEVELDB
    auto out = std::unique_ptr<BalanceDb>(new BalanceDb());
    leveldb::Options opt;
    opt.create_if_missing = writable;
    leveldb::DB* raw = nullptr;
    const leveldb::Status s = leveldb::DB::Open(opt, dir, &raw);
    if (!s.ok() || raw == nullptr) {
        fail(writable ? "cannot create balance database" : "balance database not found (run btk balance --sync)");
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

std::uint64_t BalanceDb::get_sats(const std::string& addr) const {
#ifndef BTK_NO_LEVELDB
    std::string raw;
    if (!read_key(impl_->db.get(), key_a(addr), raw) || raw.size() != 8) {
        return 0;
    }
    return get_le64(raw);
#else
    (void)addr;
    return 0;
#endif
}

bool BalanceDb::get_outpoint(const Hash256& txid, std::uint32_t vout, std::string& addr,
                             std::uint64_t& amount) const {
#ifndef BTK_NO_LEVELDB
    std::string raw;
    if (!read_key(impl_->db.get(), key_o(txid, vout), raw)) {
        return false;
    }
    return decode_out_value(raw, addr, amount);
#else
    (void)txid;
    (void)vout;
    (void)addr;
    (void)amount;
    return false;
#endif
}

bool BalanceDb::has_metadata() const {
#ifndef BTK_NO_LEVELDB
    std::string h;
    std::string t;
    return read_key(impl_->db.get(), "Mheight", h) && h.size() == 4 &&
           read_key(impl_->db.get(), "Mtip", t) && t.size() == 32;
#else
    return false;
#endif
}

std::uint32_t BalanceDb::height() const {
#ifndef BTK_NO_LEVELDB
    std::string h;
    if (!read_key(impl_->db.get(), "Mheight", h) || h.size() != 4) {
        fail("balance database not found (run btk balance --sync)");
    }
    return get_le32(h);
#else
    return 0;
#endif
}

Hash256 BalanceDb::tip() const {
#ifndef BTK_NO_LEVELDB
    std::string t;
    if (!read_key(impl_->db.get(), "Mtip", t) || t.size() != 32) {
        fail("balance database not found (run btk balance --sync)");
    }
    Hash256 out{};
    std::memcpy(out.data(), t.data(), 32);
    return out;
#else
    return {};
#endif
}

void BalanceDb::apply(const BlockEffects& fx) {
#ifndef BTK_NO_LEVELDB
    if (!impl_->writable) {
        fail("internal: read-only database");
    }
    leveldb::WriteBatch batch;
    std::unordered_map<std::string, std::uint64_t> balances;

    auto current = [&](const std::string& addr) -> std::uint64_t {
        auto it = balances.find(addr);
        if (it != balances.end()) {
            return it->second;
        }
        const std::uint64_t v = get_sats(addr);
        balances.emplace(addr, v);
        return v;
    };

    for (const OutpointRef& sp : fx.spends) {
        std::string addr;
        std::uint64_t amount = 0;
        if (!get_outpoint(sp.txid, sp.vout, addr, amount)) {
            // Unindexed script (OP_RETURN, nonstandard, …). Not an error.
            continue;
        }
        const std::uint64_t next = (current(addr) < amount) ? 0 : (current(addr) - amount);
        balances[addr] = next;
        batch.Delete(key_o(sp.txid, sp.vout));
    }

    for (const Credit& c : fx.credits) {
        if (c.address.empty() || c.amount == 0) {
            continue;
        }
        balances[c.address] = current(c.address) + c.amount;
        batch.Put(key_o(c.txid, c.vout), encode_out_value(c.address, c.amount));
    }

    for (const auto& kv : balances) {
        const std::string ak = key_a(kv.first);
        if (kv.second == 0) {
            batch.Delete(ak);
        } else {
            std::string v;
            put_le64(v, kv.second);
            batch.Put(ak, v);
        }
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
