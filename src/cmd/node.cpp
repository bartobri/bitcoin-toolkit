#include "cmd/node.hpp"

#include "core/json_io.hpp"
#include "net/p2p.hpp"
#include "util/error.hpp"

#include <cerrno>
#include <cstdlib>
#include <string>
#include <utility>

namespace {

const char kHelp[] = R"help(btk node — Bitcoin P2P version handshake (IPv4 mainnet)

Usage:
  btk node --host HOST [--port 8333]

Connect to a Bitcoin P2P peer, send a version message (protocol
70015, user agent /Bitcoin-Toolkit:4.0.0/), print the peer's
version as a typed object, and close. Does not send verack. One
shot; not a key pipe. --host is required (no positional host).

IPv4 mainnet only (getaddrinfo AF_INET). Default port 8333. 15 s
timeout on connect and read. --host may include :port (one colon).
Combined with --port that is "port specified twice". --network is
ignored. Does not load ~/.btk/config.json.

Options:
  -h, --help             Show this help and exit
      --host HOST        IPv4 address or DNS name. Required. A
                         host:port form sets the port.
      --port PORT        TCP port. Default: 8333
      --verbose          Include raw P2P fields: addr_recv,
                         addr_trans, nonce (decimal string), and
                         services_bits
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints ip:port.

Examples:
  btk node --host seed.bitcoin.sipa.be
  btk node --host 127.0.0.1 --port 8333 --verbose
  btk node --host seed.bitcoin.sipa.be --out plain
)help";

void set_int64(JsonObject& o, const char* key, std::int64_t value) {
    o[key] = JsonValue(static_cast<double>(value));
}

JsonObject net_addr_object(const NetAddr& addr) {
    JsonObject o;
    set_uint64(o, "services", addr.services);
    set_string(o, "ip", format_net_ip(addr.ip));
    set_uint64(o, "port", addr.port);
    return o;
}

JsonObject node_object(const NodePeer& peer, bool verbose) {
    JsonObject o;
    set_string(o, "type", "node");
    set_string(o, "host", peer.host);
    set_string(o, "ip", peer.ip);
    set_uint64(o, "port", peer.port);
    set_int64(o, "protocol", peer.version.version);
    set_string(o, "user_agent", peer.version.user_agent);
    set_int64(o, "height", peer.version.start_height);

    JsonArray services;
    for (const std::string& name : service_names(peer.version.services)) {
        services.emplace_back(name);
    }
    o["services"] = JsonValue(std::move(services));

    set_bool(o, "relay", peer.version.relay);
    set_int64(o, "timestamp", peer.version.timestamp);

    if (verbose) {
        JsonObject raw;
        raw["addr_recv"] = JsonValue(net_addr_object(peer.version.addr_recv));
        raw["addr_trans"] = JsonValue(net_addr_object(peer.version.addr_trans));
        // uint64 nonce does not fit in a JSON number (IEEE double).
        set_string(raw, "nonce", std::to_string(peer.version.nonce));
        set_uint64(raw, "services_bits", peer.version.services);
        o["raw"] = JsonValue(std::move(raw));
    }
    return o;
}

bool parse_port_text(const std::string& text, std::uint16_t& port) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const unsigned long v = std::strtoul(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || errno == ERANGE || v < 1 || v > 65535) {
        return false;
    }
    port = static_cast<std::uint16_t>(v);
    return true;
}

void resolve_host_port(const Options& opts, std::string& host, std::uint16_t& port) {
    host = opts.host;
    port = opts.port_set ? opts.port : kMainnetPort;

    const std::size_t colon = host.find(':');
    if (colon != std::string::npos) {
        if (host.find(':', colon + 1) != std::string::npos) {
            throw BtkError("node", "invalid host");
        }
        if (opts.port_set) {
            throw BtkError("node", "port specified twice");
        }
        const std::string suffix = host.substr(colon + 1);
        host = host.substr(0, colon);
        if (!parse_port_text(suffix, port)) {
            throw BtkError("node", "invalid port");
        }
    }
    if (host.empty()) {
        throw BtkError("node", "missing host");
    }
}

class NodeCommand : public Command {
public:
    const char* name() const override { return "node"; }
    const char* summary() const override { return "Handshake a Bitcoin P2P peer (IPv4 mainnet)"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "host", true);
        spec.add(0, "port", true);
        spec.add(0, "verbose", false);
    }

    bool is_generator(const Options&) const override { return true; }

    void init(Options& opts) override {
        if (!opts.positionals.empty()) {
            throw BtkError("node", "provide input on stdin");
        }
        if (opts.stream) {
            throw BtkError("node", "node does not stream");
        }
        if (opts.count_set) {
            throw BtkError("node", "unknown option '--count'");
        }
        if (opts.host.empty()) {
            throw BtkError("node", "missing host");
        }
        resolve_host_port(opts, host_, port_);
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>&) override {
        return {node_object(handshake_version(host_, port_), opts.verbose)};
    }

private:
    std::string host_;
    std::uint16_t port_ = kMainnetPort;
};

}  // namespace

std::unique_ptr<Command> make_node_command() {
    return std::make_unique<NodeCommand>();
}
