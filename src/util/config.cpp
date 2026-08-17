#include "util/config.hpp"

#include "core/json_io.hpp"
#include "util/error.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace {

[[noreturn]] void bad_config(const std::string& command) {
    throw BtkError(command.empty() ? "balance" : command, "invalid config file");
}

[[noreturn]] void fail_write(const std::string& command) {
    throw BtkError(command.empty() ? "config" : command, "cannot write config file");
}

bool has_field(const JsonObject& o, const char* key) {
    return o.find(key) != o.end();
}

void validate_rpc(const JsonObject& root, const std::string& command) {
    auto rpc_it = root.find("rpc");
    if (rpc_it == root.end()) {
        return;
    }
    if (!rpc_it->second.is<JsonObject>()) {
        bad_config(command);
    }
    const JsonObject& rpc = rpc_it->second.get<JsonObject>();
    if (has_field(rpc, "host") && !rpc.at("host").is<std::string>()) {
        bad_config(command);
    }
    if (has_field(rpc, "port")) {
        if (!rpc.at("port").is<double>()) {
            bad_config(command);
        }
        const double d = rpc.at("port").get<double>();
        if (d != static_cast<double>(static_cast<long>(d)) || d < 1 || d > 65535) {
            bad_config(command);
        }
    }
    if (has_field(rpc, "auth") && !rpc.at("auth").is<std::string>()) {
        bad_config(command);
    }
}

LoadedConfig extract_loaded(const JsonObject& root) {
    LoadedConfig cfg;
    cfg.present = true;
    auto rpc_it = root.find("rpc");
    if (rpc_it == root.end()) {
        return cfg;
    }
    const JsonObject& rpc = rpc_it->second.get<JsonObject>();
    if (has_field(rpc, "host")) {
        cfg.rpc_host = rpc.at("host").get<std::string>();
    }
    if (has_field(rpc, "port")) {
        cfg.rpc_port = static_cast<std::uint16_t>(rpc.at("port").get<double>());
    }
    if (has_field(rpc, "auth")) {
        cfg.rpc_auth = rpc.at("auth").get<std::string>();
    }
    return cfg;
}

std::string parent_dir(const std::string& path) {
    const auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return {};
    }
    if (pos == 0) {
        return "/";
    }
    return path.substr(0, pos);
}

bool mkdir_p(const std::string& dir) {
    if (dir.empty() || dir == "." || dir == "/") {
        return true;
    }
    struct stat st {};
    if (::stat(dir.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    const std::string parent = parent_dir(dir);
    if (!parent.empty() && !mkdir_p(parent)) {
        return false;
    }
    if (::mkdir(dir.c_str(), 0700) != 0 && errno != EEXIST) {
        return false;
    }
    return true;
}

bool write_all(int fd, const std::string& body) {
    std::size_t off = 0;
    while (off < body.size()) {
        const ssize_t n = ::write(fd, body.data() + off, body.size() - off);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        off += static_cast<std::size_t>(n);
    }
    return true;
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

ConfigDocument load_config_document(const Options& opts) {
    ConfigDocument doc;
    const std::string path = resolve_config_path(opts);
    std::ifstream in(path);
    if (!in) {
        return doc;
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
    doc.root = v.get<JsonObject>();
    validate_rpc(doc.root, opts.command);
    doc.present = true;
    return doc;
}

LoadedConfig load_config(const Options& opts) {
    const ConfigDocument doc = load_config_document(opts);
    if (!doc.present) {
        return {};
    }
    return extract_loaded(doc.root);
}

void save_config_document(const Options& opts, const JsonObject& root) {
    const std::string path = resolve_config_path(opts);
    const std::string parent = parent_dir(path);
    if (!parent.empty() && !mkdir_p(parent)) {
        throw BtkError(opts.command.empty() ? "config" : opts.command,
                       "cannot create config directory");
    }
    const std::string tmp = path + ".tmp";
    const int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        fail_write(opts.command);
    }
    if (::fchmod(fd, 0600) != 0) {
        ::close(fd);
        ::unlink(tmp.c_str());
        fail_write(opts.command);
    }
    const std::string body = json_pretty(JsonValue(root)) + "\n";
    if (!write_all(fd, body)) {
        ::close(fd);
        ::unlink(tmp.c_str());
        fail_write(opts.command);
    }
    if (::close(fd) != 0) {
        ::unlink(tmp.c_str());
        fail_write(opts.command);
    }
    if (::rename(tmp.c_str(), path.c_str()) != 0) {
        ::unlink(tmp.c_str());
        fail_write(opts.command);
    }
}
