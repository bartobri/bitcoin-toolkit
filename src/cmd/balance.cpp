#include "cmd/balance.hpp"

#include "chain/balance_db.hpp"
#include "chain/indexer.hpp"
#include "chain/script.hpp"
#include "chain/transaction.hpp"
#include "core/hex.hpp"
#include "core/json_io.hpp"
#include "net/jsonrpc.hpp"
#include "util/config.hpp"
#include "util/error.hpp"
#include "util/interrupt.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace {

const char kHelp[] = R"help(btk balance — local address → satoshi index

Usage:
  btk balance
  btk balance --sync [--host H] [--port P] [--rpc-auth USER:PASS]
                     [--force]

Query a local address-to-satoshi index, or synchronize it from
Bitcoin Core JSON-RPC. The index always lives at ~/.btk/balance.
Requires LevelDB at build time. Mainnet only.

Query is a transformer on stdin (no positional addresses). Items
are a typed address object (data), a typed balance object
(address), or a bare Base58Check / bech32 address. --from address
forces the bare-line parse. Leftover text is an error, not a hash.
Empty stdin is empty stdout, exit 0. A missing address is sats: 0.
A missing database is "balance database not found (run btk balance
--sync)". Query is read-only.

--sync walks Core JSON-RPC (getblockcount, getblockhash, getblock
hex). Missing or empty DB: walk 0…tip. Valid DB: walk
Mheight+1…tip (already at tip → complete, exit 0). Junk or a reorg:
rebuild with --sync --force. --force wipes the directory and walks
from genesis. Ctrl-C / SIGTERM abort within ~200 ms; the next
--sync continues from the last saved height. Progress is on stderr.

No cookie file. Use --rpc-auth user:pass or config rpc.auth.
--host / --port default to config rpc.host / rpc.port or
127.0.0.1 / 8332. A first mainnet sync needs a node that can serve
every historical block and can take days; later runs are
incremental.

Options:
  -h, --help             Show this help and exit
      --sync             Create the index or catch it up from RPC
      --force            With --sync, wipe ~/.btk/balance and walk
                         from genesis
      --host HOST        RPC host. Default: config rpc.host or
                         127.0.0.1. A host:port form sets the port.
      --port PORT        RPC port. Default: config rpc.port or 8332
      --rpc-auth USER:PASS
                         HTTP Basic credentials. Default: config
                         rpc.auth. No cookie file.
      --from address     Force bare stdin lines as addresses
      --config PATH      Config file. Default: $BTK_CONFIG, else
                         ~/.btk/config.json
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the satoshi count.
      --in FORMAT        auto (default), ndjson, json, or plain

Examples:
  btk balance --sync
  printf '%s' 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa | btk balance
  btk privkey --new | btk address | btk balance
)help";

[[noreturn]] void fail(const std::string& message) {
    throw BtkError("balance", message);
}

std::string object_string(const JsonObject& item, const char* key) {
    auto it = item.find(key);
    if (it == item.end() || !it->second.is<std::string>()) {
        return {};
    }
    return it->second.get<std::string>();
}

std::string item_address(const JsonObject& item) {
    if (is_bare(item)) {
        return bare_text(item);
    }
    const std::string type = object_string(item, "type");
    if (type == "address") {
        const std::string data = object_string(item, "data");
        if (data.empty()) {
            fail("expected an address");
        }
        return data;
    }
    if (type == "balance") {
        const std::string addr = object_string(item, "address");
        if (addr.empty()) {
            fail("expected an address");
        }
        return addr;
    }
    fail("expected an address");
}

JsonObject balance_object(const std::string& addr, std::uint64_t sats) {
    JsonObject o;
    set_string(o, "type", "balance");
    set_string(o, "address", addr);
    set_uint64(o, "sats", sats);
    return o;
}

Hash256 hash_from_display(const std::string& hex) {
    std::vector<std::uint8_t> b;
    try {
        b = hex_decode(hex);
    } catch (const BtkError&) {
        fail("invalid rpc response");
    }
    if (b.size() != 32) {
        fail("invalid rpc response");
    }
    std::reverse(b.begin(), b.end());
    Hash256 out{};
    std::copy(b.begin(), b.end(), out.begin());
    return out;
}

std::string hash_to_display(const Hash256& h) {
    Hash256 rev = h;
    std::reverse(rev.begin(), rev.end());
    return hex_encode(rev);
}

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t max) : max_(max) {}

    void push(T item) {
        std::unique_lock<std::mutex> lock(mu_);
        while (!(q_.size() < max_ || closed_ || err_)) {
            cv_not_full_.wait_for(lock, std::chrono::milliseconds(200));
            if (stop_requested()) {
                closed_ = true;
                cv_not_empty_.notify_all();
                cv_not_full_.notify_all();
                return;
            }
        }
        if (err_) {
            std::rethrow_exception(err_);
        }
        if (closed_) {
            return;
        }
        q_.push(std::move(item));
        cv_not_empty_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mu_);
        while (!(!q_.empty() || closed_ || err_)) {
            cv_not_empty_.wait_for(lock, std::chrono::milliseconds(200));
            if (stop_requested() && q_.empty()) {
                closed_ = true;
                cv_not_empty_.notify_all();
                cv_not_full_.notify_all();
                return false;
            }
        }
        if (err_) {
            std::rethrow_exception(err_);
        }
        if (q_.empty()) {
            return false;
        }
        item = std::move(q_.front());
        q_.pop();
        cv_not_full_.notify_one();
        return true;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mu_);
        closed_ = true;
        cv_not_empty_.notify_all();
        cv_not_full_.notify_all();
    }

    void fail(std::exception_ptr e) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!err_) {
            err_ = std::move(e);
        }
        closed_ = true;
        cv_not_empty_.notify_all();
        cv_not_full_.notify_all();
    }

private:
    std::size_t max_;
    std::queue<T> q_;
    std::mutex mu_;
    std::condition_variable cv_not_empty_;
    std::condition_variable cv_not_full_;
    bool closed_ = false;
    std::exception_ptr err_;
};

struct RawBlock {
    std::uint32_t height = 0;
    std::string hash_hex;
    std::vector<std::uint8_t> raw;
};

void print_progress(std::uint32_t height, std::uint32_t tip) {
    const std::uint32_t total = tip + 1;
    const std::uint32_t done = height + 1;
    const double pct = 100.0 * static_cast<double>(done) / static_cast<double>(total);
    std::fprintf(stderr, "\rsyncing: height %u/%u (%.1f%%)", height, tip, pct);
    std::fflush(stderr);
}

void print_complete(std::uint32_t height) {
    std::fprintf(stderr, "\ncomplete: height %u\n", height);
}

void run_sync(const std::string& dir, const std::string& host, std::uint16_t port,
              const std::string& auth, bool incremental, std::uint32_t start_height,
              const Hash256* expected_tip) {
    JsonRpc rpc(host, port, auth);
    const std::uint32_t tip = rpc_getblockcount(rpc);

    if (incremental) {
        if (start_height > tip) {
            fail("reorg detected; rebuild with --sync --force");
        }
        const Hash256 at = hash_from_display(rpc_getblockhash(rpc, start_height));
        if (expected_tip == nullptr || at != *expected_tip) {
            fail("reorg detected; rebuild with --sync --force");
        }
        if (start_height == tip) {
            print_complete(tip);
            return;
        }
    }

    const std::uint32_t first = incremental ? start_height + 1 : 0;
    auto db = BalanceDb::open(dir, true);

    BoundedQueue<RawBlock> raw_q(8);
    BoundedQueue<BlockEffects> fx_q(100);
    std::exception_ptr worker_err;
    std::uint32_t last_applied = incremental ? start_height : 0;
    bool applied_any = false;

    std::thread fetch([&] {
        try {
            for (std::uint32_t h = first; h <= tip; ++h) {
                if (stop_requested()) {
                    break;
                }
                RawBlock rb;
                rb.height = h;
                rb.hash_hex = rpc_getblockhash(rpc, h);
                rb.raw = rpc_getblock(rpc, rb.hash_hex);
                raw_q.push(std::move(rb));
                if (h == tip) {
                    break;
                }
            }
            raw_q.close();
        } catch (const BtkError& err) {
            if (stop_requested() || std::string(err.what()) == "interrupted") {
                raw_q.close();
                return;
            }
            raw_q.fail(std::current_exception());
            fx_q.fail(std::current_exception());
        } catch (...) {
            raw_q.fail(std::current_exception());
            fx_q.fail(std::current_exception());
        }
    });

    std::thread parse([&] {
        try {
            RawBlock rb;
            while (raw_q.pop(rb)) {
                const Block block = parse_block(rb.raw);
                if (hash_to_display(block_hash(block)) != rb.hash_hex) {
                    fail("invalid rpc response");
                }
                fx_q.push(effects_from_block(block, rb.height));
            }
            fx_q.close();
        } catch (...) {
            fx_q.fail(std::current_exception());
        }
    });

    std::thread write([&] {
        try {
            BlockEffects fx;
            while (fx_q.pop(fx)) {
                db->apply(fx);
                last_applied = fx.height;
                applied_any = true;
                print_progress(fx.height, tip);
            }
        } catch (...) {
            worker_err = std::current_exception();
            raw_q.fail(worker_err);
            fx_q.fail(worker_err);
        }
    });

    fetch.join();
    parse.join();
    write.join();
    if (worker_err) {
        std::rethrow_exception(worker_err);
    }
    if (stop_requested()) {
        if (applied_any || incremental) {
            std::fprintf(stderr, "\ninterrupted: height %u\n", last_applied);
        } else {
            std::fprintf(stderr, "\ninterrupted\n");
        }
        fail("interrupted");
    }
    print_complete(tip);
}

class BalanceCommand : public Command {
public:
    const char* name() const override { return "balance"; }
    const char* summary() const override { return "Sync or query a local address-balance index"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "sync", false);
        spec.add(0, "force", false);
        spec.add(0, "host", true);
        spec.add(0, "port", true);
        spec.add(0, "rpc-auth", true);
        spec.add(0, "from", true);
    }

    bool is_generator(const Options& opts) const override { return opts.sync; }

    void init(Options& opts) override {
        if (!kHaveLeveldb) {
            fail("this build was compiled without LevelDB (install libleveldb-dev and rebuild)");
        }
        if (!opts.positionals.empty()) {
            fail("provide input on stdin");
        }
        if (opts.count_set) {
            fail("unknown option '--count'");
        }
        if (opts.stream && opts.sync) {
            fail("balance does not stream");
        }
        if (opts.force && !opts.sync) {
            fail("--force requires --sync");
        }
        if (!opts.from.empty()) {
            if (opts.sync) {
                fail("cannot combine --sync and --from");
            }
            if (opts.from != "address") {
                fail("invalid --from");
            }
        }

        const LoadedConfig cfg = load_config(opts);
        if (opts.sync) {
            host_ = opts.host.empty() ? cfg.rpc_host : opts.host;
            port_ = opts.port_set ? opts.port : cfg.rpc_port;
            auth_ = opts.rpc_auth.empty() ? cfg.rpc_auth : opts.rpc_auth;
            const std::size_t colon = host_.find(':');
            if (colon != std::string::npos) {
                if (host_.find(':', colon + 1) != std::string::npos) {
                    fail("invalid host");
                }
                if (opts.port_set) {
                    fail("port specified twice");
                }
                const std::string suffix = host_.substr(colon + 1);
                host_ = host_.substr(0, colon);
                char* end = nullptr;
                errno = 0;
                const unsigned long v = std::strtoul(suffix.c_str(), &end, 10);
                if (end == suffix.c_str() || *end != '\0' || errno == ERANGE || v < 1 || v > 65535) {
                    fail("invalid port");
                }
                port_ = static_cast<std::uint16_t>(v);
            }
            if (host_.empty()) {
                fail("invalid host");
            }
        }
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>& item) override {
        const std::string dir = default_balance_dir();
        if (opts.sync) {
            do_sync(dir, opts.force);
            return {};
        }
        if (!item) {
            return {};
        }
        const std::string addr = item_address(*item);
        if (!is_mainnet_address(addr)) {
            fail("not a bitcoin address");
        }
        if (inspect_index(dir) != IndexState::Valid) {
            fail("balance database not found (run btk balance --sync)");
        }
        auto db = BalanceDb::open(dir, false);
        return {balance_object(addr, db->get_sats(addr))};
    }

private:
    void do_sync(const std::string& dir, bool force) {
        const IndexState st = inspect_index(dir);
        if (force) {
            destroy_index(dir);
            ensure_btk_home();
            run_sync(dir, host_, port_, auth_, false, 0, nullptr);
            return;
        }
        if (st == IndexState::Valid) {
            auto db = BalanceDb::open(dir, false);
            const std::uint32_t h = db->height();
            const Hash256 tip = db->tip();
            db.reset();
            run_sync(dir, host_, port_, auth_, true, h, &tip);
            return;
        }
        if (st == IndexState::Junk) {
            fail("balance database exists; rebuild with --sync --force");
        }
        ensure_btk_home();
        run_sync(dir, host_, port_, auth_, false, 0, nullptr);
    }

    std::string host_;
    std::uint16_t port_ = 8332;
    std::string auth_;
};

}  // namespace

std::unique_ptr<Command> make_balance_command() {
    return std::make_unique<BalanceCommand>();
}
