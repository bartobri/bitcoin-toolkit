#include "cmd/inflow.hpp"

#include "chain/inflow_db.hpp"
#include "chain/script.hpp"
#include "chain/sync.hpp"
#include "core/json_io.hpp"
#include "util/config.hpp"
#include "util/error.hpp"

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <optional>
#include <utility>

namespace {

const char kHelp[] = R"help(btk inflow — local address → lifetime-received index

Usage:
  btk inflow
  btk inflow --sync [--host H] [--port P] [--rpc-auth USER:PASS]
                    [--force]

Query a local lifetime-received index, or synchronize it from
Bitcoin Core JSON-RPC. The index always lives at ~/.btk/inflow.
Requires LevelDB at build time. Mainnet only.

Query is a transformer on stdin (no positional addresses). Items
are a typed address object (data), a typed inflow or balance
object (address), or a bare Base58Check / bech32 address.
--from address forces the bare-line parse. Leftover text is an
error, not a hash. Empty stdin is empty stdout, exit 0. A missing
address is sats: 0, count: 0, and last: "". --skip-zero drops rows
with sats: 0 (empty stdout is still exit 0). A missing database
is "inflow database not found (run btk inflow --sync)". Query is
read-only.

sats is lifetime received and is never reduced by a spend. count
is the number of transactions that credited the address. last is
the UTC time of the newest credit (YYYY-MM-DD HH:MM:SS UTC), or
"" when the address has never been seen.

--since DURATION is query-only. The cutoff is now minus the
duration, or zero when that duration is longer than now. When
the newest credit is older than the cutoff, the row is reported
as sats: 0 and count: 0; last is unchanged. A newest credit at
or after the cutoff keeps the lifetime total. An address never
seen is unchanged. --skip-zero runs after that zeroing. The
duration is an integer of at least 1, an optional single space,
and one lowercase unit: h, hour, hours, d, day, days, w, week,
weeks, mo, month, months, y, yr, year, or years. A bad duration
is "invalid --since".

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
      --force            With --sync, wipe ~/.btk/inflow and walk
                         from genesis
      --host HOST        RPC host. Default: config rpc.host or
                         127.0.0.1. A host:port form sets the port.
      --port PORT        RPC port. Default: config rpc.port or 8332
      --rpc-auth USER:PASS
                         HTTP Basic credentials. Default: config
                         rpc.auth. No cookie file.
      --from address     Force bare stdin lines as addresses
      --source           Include the input item as a source object
      --skip-zero        Omit addresses with sats: 0
      --since DURATION   Zero sats and count when the newest credit
                         is older than DURATION. Query only.
      --config PATH      Config file. Default: $BTK_CONFIG, else
                         ~/.btk/config.json
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the satoshi count.
      --in FORMAT        auto (default), ndjson, json, or plain

Examples:
  btk inflow --sync
  printf '%s' 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa | btk inflow
  btk privkey --new | btk address | btk inflow
  btk privkey --new | btk address | btk inflow --skip-zero
  btk privkey --new | btk address --source | btk inflow --source
  printf '%s' 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa | btk inflow --since 1y
)help";

[[noreturn]] void fail(const std::string& message) {
    throw BtkError("inflow", message);
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
    if (type == "inflow" || type == "balance") {
        const std::string addr = object_string(item, "address");
        if (addr.empty()) {
            fail("expected an address");
        }
        return addr;
    }
    fail("expected an address");
}

JsonObject synthesized_address_source(const std::string& addr) {
    JsonObject o;
    set_string(o, "type", "address");
    if (const auto style = classify_mainnet_address(addr)) {
        set_string(o, "style", *style);
    }
    set_string(o, "network", "mainnet");
    set_string(o, "data", addr);
    return o;
}

std::optional<JsonObject> source_from_item(const JsonObject& item, bool want) {
    if (!want) {
        return std::nullopt;
    }
    if (is_bare(item)) {
        return synthesized_address_source(bare_text(item));
    }
    return item;
}

std::uint64_t parse_since_seconds(const std::string& spec) {
    if (spec.empty() || !std::isdigit(static_cast<unsigned char>(spec[0]))) {
        fail("invalid --since");
    }
    std::size_t i = 0;
    while (i < spec.size() && std::isdigit(static_cast<unsigned char>(spec[i]))) {
        ++i;
    }
    std::size_t unit_at = i;
    if (i < spec.size() && spec[i] == ' ') {
        ++unit_at;
        if (unit_at >= spec.size() || spec[unit_at] == ' ') {
            fail("invalid --since");
        }
    }
    if (unit_at >= spec.size()) {
        fail("invalid --since");
    }
    const std::string number = spec.substr(0, i);
    const std::string unit = spec.substr(unit_at);

    errno = 0;
    char* end = nullptr;
    const unsigned long long n = std::strtoull(number.c_str(), &end, 10);
    if (end == number.c_str() || *end != '\0' || errno == ERANGE || n < 1) {
        fail("invalid --since");
    }

    std::uint64_t scale = 0;
    if (unit == "h" || unit == "hour" || unit == "hours") {
        scale = 3600;
    } else if (unit == "d" || unit == "day" || unit == "days") {
        scale = 86400;
    } else if (unit == "w" || unit == "week" || unit == "weeks") {
        scale = 7ull * 86400ull;
    } else if (unit == "mo" || unit == "month" || unit == "months") {
        scale = 30ull * 86400ull;
    } else if (unit == "y" || unit == "yr" || unit == "year" || unit == "years") {
        scale = 365ull * 86400ull;
    } else {
        fail("invalid --since");
    }
    if (n > std::numeric_limits<std::uint64_t>::max() / scale) {
        fail("invalid --since");
    }
    return static_cast<std::uint64_t>(n) * scale;
}

std::string format_last(std::uint32_t unix_time) {
    if (unix_time == 0) {
        return {};
    }
    const std::time_t t = static_cast<std::time_t>(unix_time);
    std::tm tm{};
    if (gmtime_r(&t, &tm) == nullptr) {
        return {};
    }
    char buf[32];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tm) == 0) {
        return {};
    }
    return buf;
}

JsonObject inflow_object(const std::string& addr, const InflowRow& row,
                         const std::optional<JsonObject>& source) {
    JsonObject o;
    set_string(o, "type", "inflow");
    set_string(o, "address", addr);
    set_uint64(o, "sats", row.sats);
    set_uint64(o, "count", row.count);
    set_string(o, "last", format_last(row.last));
    if (source) {
        o["source"] = JsonValue(*source);
    }
    return o;
}

class InflowCommand : public Command {
public:
    const char* name() const override { return "inflow"; }
    const char* summary() const override { return "Sync or query a local address-inflow index"; }
    const char* help() const override { return kHelp; }
    bool hidden() const override { return true; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "sync", false);
        spec.add(0, "force", false);
        spec.add(0, "host", true);
        spec.add(0, "port", true);
        spec.add(0, "rpc-auth", true);
        spec.add(0, "from", true);
        spec.add(0, "source", false);
        spec.add(0, "skip-zero", false);
        spec.add(0, "since", true);
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
            fail("inflow does not stream");
        }
        if (opts.force && !opts.sync) {
            fail("--force requires --sync");
        }
        if (opts.source && opts.sync) {
            fail("cannot combine --sync and --source");
        }
        if (opts.skip_zero && opts.sync) {
            fail("cannot combine --sync and --skip-zero");
        }
        if (!opts.since.empty() && opts.sync) {
            fail("cannot combine --sync and --since");
        }
        if (!opts.since.empty()) {
            const std::uint64_t secs = parse_since_seconds(opts.since);
            const std::time_t now = std::time(nullptr);
            const std::uint64_t now_u = now > 0 ? static_cast<std::uint64_t>(now) : 0;
            since_cutoff_ = secs > now_u ? 0 : now_u - secs;
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
        const std::string dir = default_inflow_dir();
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
        if (!db_) {
            db_ = InflowDb::open(dir, false);
            if (!db_->has_metadata()) {
                db_.reset();
                fail("inflow database not found (run btk inflow --sync)");
            }
        }
        InflowRow row = db_->get_row(addr);
        if (since_cutoff_ && row.last != 0 &&
            static_cast<std::uint64_t>(row.last) < *since_cutoff_) {
            row.sats = 0;
            row.count = 0;
        }
        if (opts.skip_zero && row.sats == 0) {
            return {};
        }
        return {inflow_object(addr, row, source_from_item(*item, opts.source))};
    }

private:
    void do_sync(const std::string& dir, bool force) {
        const IndexState st = inspect_index(dir);
        if (force) {
            destroy_index(dir, "inflow");
            ensure_btk_home("inflow");
            auto db = InflowDb::open(dir, true);
            walk_rpc_blocks("inflow", host_, port_, auth_, false, 0, nullptr,
                            [&](const BlockEffects& fx) { db->apply(fx); });
            return;
        }
        if (st == IndexState::Valid) {
            auto db = InflowDb::open(dir, false);
            const std::uint32_t h = db->height();
            const Hash256 tip = db->tip();
            db.reset();
            auto wdb = InflowDb::open(dir, true);
            walk_rpc_blocks("inflow", host_, port_, auth_, true, h, &tip,
                            [&](const BlockEffects& fx) { wdb->apply(fx); });
            return;
        }
        if (st == IndexState::Junk) {
            fail("inflow database exists; rebuild with --sync --force");
        }
        ensure_btk_home("inflow");
        auto db = InflowDb::open(dir, true);
        walk_rpc_blocks("inflow", host_, port_, auth_, false, 0, nullptr,
                        [&](const BlockEffects& fx) { db->apply(fx); });
    }

    std::string host_;
    std::uint16_t port_ = 8332;
    std::string auth_;
    std::unique_ptr<InflowDb> db_;
    std::optional<std::uint64_t> since_cutoff_;
};

}  // namespace

std::unique_ptr<Command> make_inflow_command() {
    return std::make_unique<InflowCommand>();
}
