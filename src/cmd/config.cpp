#include "cmd/config.hpp"

#include "core/json_io.hpp"
#include "util/config.hpp"
#include "util/error.hpp"

#include <cerrno>
#include <cstdlib>
#include <string>
#include <utility>

namespace {

const char kHelp[] = R"help(btk config — defaults for RPC

Usage:
  btk config set <key>=<value>
  btk config unset <key>
  btk config get <key>
  btk config dump

Keys: rpc.host, rpc.port, rpc.auth
File: ~/.btk/config.json (or --config / $BTK_CONFIG), mode 0600.
dump and get redact rpc.auth as ********.
)help";

constexpr const char kRedacted[] = "********";

enum class Verb { Set, Get, Unset, Dump };

enum class Key { Host, Port, Auth };

[[noreturn]] void fail(const std::string& message) {
    throw BtkError("config", message);
}

const char* key_name(Key key) {
    switch (key) {
        case Key::Host:
            return "rpc.host";
        case Key::Port:
            return "rpc.port";
        case Key::Auth:
            return "rpc.auth";
    }
    return "";
}

const char* rpc_field(Key key) {
    switch (key) {
        case Key::Host:
            return "host";
        case Key::Port:
            return "port";
        case Key::Auth:
            return "auth";
    }
    return "";
}

Key parse_key(const std::string& name) {
    if (name == "rpc.host") {
        return Key::Host;
    }
    if (name == "rpc.port") {
        return Key::Port;
    }
    if (name == "rpc.auth") {
        return Key::Auth;
    }
    fail("unknown config key '" + name + "'");
}

std::uint16_t parse_port_value(const std::string& text) {
    if (text.empty()) {
        fail("invalid rpc.port");
    }
    char* end = nullptr;
    errno = 0;
    const unsigned long v = std::strtoul(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || errno == ERANGE || v < 1 || v > 65535) {
        fail("invalid rpc.port");
    }
    return static_cast<std::uint16_t>(v);
}

const JsonObject* rpc_object(const JsonObject& root) {
    auto it = root.find("rpc");
    if (it == root.end()) {
        return nullptr;
    }
    if (!it->second.is<JsonObject>()) {
        fail("invalid config file");
    }
    return &it->second.get<JsonObject>();
}

JsonObject* rpc_object(JsonObject& root) {
    return const_cast<JsonObject*>(rpc_object(static_cast<const JsonObject&>(root)));
}

JsonObject& ensure_rpc(JsonObject& root) {
    JsonObject* rpc = rpc_object(root);
    if (rpc != nullptr) {
        return *rpc;
    }
    root["rpc"] = JsonValue(JsonObject{});
    return root["rpc"].get<JsonObject>();
}

bool has_key(const JsonObject& root, Key key) {
    const JsonObject* rpc = rpc_object(root);
    if (rpc == nullptr) {
        return false;
    }
    return rpc->find(rpc_field(key)) != rpc->end();
}

void drop_empty_rpc(JsonObject& root) {
    auto it = root.find("rpc");
    if (it != root.end() && it->second.is<JsonObject>() && it->second.get<JsonObject>().empty()) {
        root.erase(it);
    }
}

std::string format_plain(Key key, const JsonValue& value) {
    if (key == Key::Auth) {
        return kRedacted;
    }
    if (value.is<std::string>()) {
        return value.get<std::string>();
    }
    if (value.is<double>()) {
        return std::to_string(static_cast<unsigned long long>(value.get<double>()));
    }
    return json_min(value);
}

void put_dotted(JsonObject& o, Key key, const JsonValue& raw) {
    if (key == Key::Auth) {
        set_string(o, "rpc.auth", kRedacted);
        return;
    }
    if (key == Key::Port) {
        set_uint64(o, "rpc.port", static_cast<std::uint64_t>(raw.get<double>()));
        return;
    }
    set_string(o, key_name(key), raw.get<std::string>());
}

JsonObject dump_object(const JsonObject& root) {
    JsonObject o;
    set_string(o, "type", "config");
    static const Key kOrder[] = {Key::Host, Key::Port, Key::Auth};
    const JsonObject* rpc = nullptr;
    auto it = root.find("rpc");
    if (it != root.end() && it->second.is<JsonObject>()) {
        rpc = &it->second.get<JsonObject>();
    }
    if (rpc == nullptr) {
        return o;
    }
    for (Key key : kOrder) {
        auto field = rpc->find(rpc_field(key));
        if (field == rpc->end()) {
            continue;
        }
        put_dotted(o, key, field->second);
    }
    return o;
}

class ConfigCommand : public Command {
public:
    const char* name() const override { return "config"; }
    const char* summary() const override { return "Get and set defaults (RPC)"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec&) const override {}

    bool is_generator(const Options&) const override { return true; }

    void init(Options& opts) override {
        if (opts.stream) {
            fail("config does not stream");
        }
        if (opts.count_set) {
            fail("unknown option '--count'");
        }
        if (opts.positionals.empty()) {
            fail("expected set, get, unset, or dump");
        }
        const std::string& verb = opts.positionals[0];
        if (verb == "set") {
            verb_ = Verb::Set;
            if (opts.positionals.size() < 2) {
                fail("expected key=value");
            }
            if (opts.positionals.size() > 2) {
                fail("unexpected argument");
            }
            const std::string& spec = opts.positionals[1];
            const auto eq = spec.find('=');
            if (eq == std::string::npos) {
                fail("expected key=value");
            }
            key_ = parse_key(spec.substr(0, eq));
            value_ = spec.substr(eq + 1);
            if (key_ == Key::Port) {
                port_ = parse_port_value(value_);
            }
        } else if (verb == "get") {
            verb_ = Verb::Get;
            if (opts.positionals.size() < 2) {
                fail("expected a config key");
            }
            if (opts.positionals.size() > 2) {
                fail("unexpected argument");
            }
            key_ = parse_key(opts.positionals[1]);
        } else if (verb == "unset") {
            verb_ = Verb::Unset;
            if (opts.positionals.size() < 2) {
                fail("expected a config key");
            }
            if (opts.positionals.size() > 2) {
                fail("unexpected argument");
            }
            key_ = parse_key(opts.positionals[1]);
        } else if (verb == "dump") {
            verb_ = Verb::Dump;
            if (opts.positionals.size() > 1) {
                fail("unexpected argument");
            }
        } else {
            fail("unknown config verb '" + verb + "'");
        }
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>&) override {
        switch (verb_) {
            case Verb::Set:
                return do_set(opts);
            case Verb::Get:
                return do_get(opts);
            case Verb::Unset:
                return do_unset(opts);
            case Verb::Dump:
                return do_dump(opts);
        }
        return {};
    }

private:
    std::vector<JsonObject> do_set(const Options& opts) {
        ConfigDocument doc = load_config_document(opts);
        JsonObject& rpc = ensure_rpc(doc.root);
        if (key_ == Key::Port) {
            set_uint64(rpc, "port", port_);
        } else if (key_ == Key::Host) {
            set_string(rpc, "host", value_);
        } else {
            set_string(rpc, "auth", value_);
        }
        save_config_document(opts, doc.root);
        return {};
    }

    std::vector<JsonObject> do_get(const Options& opts) {
        const ConfigDocument doc = load_config_document(opts);
        if (!doc.present || !has_key(doc.root, key_)) {
            fail("no such key");
        }
        const JsonObject* rpc = rpc_object(doc.root);
        const JsonValue& raw = rpc->at(rpc_field(key_));
        if (opts.out == OutFormat::Plain) {
            JsonObject o;
            set_string(o, "type", "config");
            set_string(o, "data", format_plain(key_, raw));
            return {std::move(o)};
        }
        JsonObject o;
        set_string(o, "type", "config");
        put_dotted(o, key_, raw);
        return {std::move(o)};
    }

    std::vector<JsonObject> do_unset(const Options& opts) {
        ConfigDocument doc = load_config_document(opts);
        if (!doc.present || !has_key(doc.root, key_)) {
            fail("no such key");
        }
        JsonObject* rpc = rpc_object(doc.root);
        rpc->erase(rpc_field(key_));
        drop_empty_rpc(doc.root);
        save_config_document(opts, doc.root);
        return {};
    }

    std::vector<JsonObject> do_dump(const Options& opts) {
        const ConfigDocument doc = load_config_document(opts);
        if (!doc.present) {
            JsonObject o;
            set_string(o, "type", "config");
            return {std::move(o)};
        }
        return {dump_object(doc.root)};
    }

    Verb verb_ = Verb::Dump;
    Key key_ = Key::Host;
    std::string value_;
    std::uint16_t port_ = 0;
};

}  // namespace

std::unique_ptr<Command> make_config_command() {
    return std::make_unique<ConfigCommand>();
}
