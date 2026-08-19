#include "chain/sync.hpp"

#include "chain/indexer.hpp"
#include "chain/transaction.hpp"
#include "core/hex.hpp"
#include "net/jsonrpc.hpp"
#include "util/error.hpp"
#include "util/interrupt.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <exception>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace {

[[noreturn]] void fail(const char* command, const std::string& message) {
    throw BtkError(command, message);
}

Hash256 hash_from_display(const char* command, const std::string& hex) {
    std::vector<std::uint8_t> b;
    try {
        b = hex_decode(hex);
    } catch (const BtkError&) {
        fail(command, "invalid rpc response");
    }
    if (b.size() != 32) {
        fail(command, "invalid rpc response");
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

void reprefix(const char* command, const std::exception_ptr& e) {
    try {
        std::rethrow_exception(e);
    } catch (const BtkError& err) {
        fail(command, err.what());
    }
}

}  // namespace

void walk_rpc_blocks(const char* command, const std::string& host, std::uint16_t port,
                     const std::string& auth, bool incremental, std::uint32_t start_height,
                     const Hash256* expected_tip,
                     const std::function<void(const BlockEffects&)>& apply) {
    JsonRpc rpc(host, port, auth, command);
    const std::uint32_t tip = rpc_getblockcount(rpc);

    if (incremental) {
        if (start_height > tip) {
            fail(command, "reorg detected; rebuild with --sync --force");
        }
        const Hash256 at = hash_from_display(command, rpc_getblockhash(rpc, start_height));
        if (expected_tip == nullptr || at != *expected_tip) {
            fail(command, "reorg detected; rebuild with --sync --force");
        }
        if (start_height == tip) {
            print_complete(tip);
            return;
        }
    }

    const std::uint32_t first = incremental ? start_height + 1 : 0;

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
                    fail(command, "invalid rpc response");
                }
                fx_q.push(effects_from_block(block, rb.height));
            }
            fx_q.close();
        } catch (const BtkError& err) {
            fx_q.fail(std::make_exception_ptr(BtkError(command, err.what())));
        } catch (...) {
            fx_q.fail(std::current_exception());
        }
    });

    std::thread write([&] {
        try {
            BlockEffects fx;
            while (fx_q.pop(fx)) {
                apply(fx);
                last_applied = fx.height;
                applied_any = true;
                print_progress(fx.height, tip);
            }
        } catch (const BtkError& err) {
            worker_err = std::make_exception_ptr(BtkError(command, err.what()));
            raw_q.fail(worker_err);
            fx_q.fail(worker_err);
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
        reprefix(command, worker_err);
    }
    if (stop_requested()) {
        if (applied_any || incremental) {
            std::fprintf(stderr, "\ninterrupted: height %u\n", last_applied);
        } else {
            std::fprintf(stderr, "\ninterrupted\n");
        }
        fail(command, "interrupted");
    }
    print_complete(tip);
}
