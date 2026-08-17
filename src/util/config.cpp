#include "util/config.hpp"

#include "core/json_io.hpp"
#include "util/error.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace {

[[noreturn]] void bad_config(const std::string& command) {
    throw BtkError(command.empty() ? "balance" : command, "invalid config file");
}

bool has_field(const JsonObject& o, const char* key) {
    return o.find(key) != o.end();
}

}  // namespace

std::string resolve_config_path(const Options& opts) {
    if (!opts.config_path.empty()) {
        return opts.config_path;
    }
    const char* env = std::getenv("BTK_CONFIG");
    if (env != nullptr && env[0] != '\0') {
        return env;
    }
    const char* home = std::getenv("HOME");
    if (home == nullptr || home[0] == '\0') {
        throw BtkError(opts.command, "HOME is not set");
    }
    return std::string(home) + "/.btk/config.json";
}

LoadedConfig load_config(const Options& opts) {
    LoadedConfig cfg;
    const std::string path = resolve_config_path(opts);
    std::ifstream in(path);
    if (!in) {
        return cfg;
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    JsonValue v;
    try {
        v = parse_json_value(buf.str(), opts.command);
    } catch (const BtkError&) {
        bad_config(opts.command);
    }
    if (!v.is<JsonObject>()) {
        bad_config(opts.command);
    }
    cfg.present = true;
    const JsonObject& root = v.get<JsonObject>();
    const JsonObject* rpc = nullptr;
    auto rpc_it = root.find("rpc");
    if (rpc_it != root.end()) {
        if (!rpc_it->second.is<JsonObject>()) {
            bad_config(opts.command);
        }
        rpc = &rpc_it->second.get<JsonObject>();
    }
    if (rpc != nullptr) {
        if (has_field(*rpc, "host")) {
            if (!(*rpc).at("host").is<std::string>()) {
                bad_config(opts.command);
            }
            cfg.rpc_host = (*rpc).at("host").get<std::string>();
        }
        if (has_field(*rpc, "port")) {
            if (!(*rpc).at("port").is<double>()) {
                bad_config(opts.command);
            }
            const double d = (*rpc).at("port").get<double>();
            if (d != static_cast<double>(static_cast<long>(d)) || d < 1 || d > 65535) {
                bad_config(opts.command);
            }
            cfg.rpc_port = static_cast<std::uint16_t>(d);
        }
        if (has_field(*rpc, "auth")) {
            if (!(*rpc).at("auth").is<std::string>()) {
                bad_config(opts.command);
            }
            cfg.rpc_auth = (*rpc).at("auth").get<std::string>();
        }
    }
    return cfg;
}
