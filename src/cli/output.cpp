#include "cli/output.hpp"

#include "version.hpp"

#include <cstdio>
#include <iostream>
#include <sstream>

std::string primary_string(const JsonObject& obj) {
    auto type_it = obj.find("type");
    const std::string type =
        (type_it != obj.end() && type_it->second.is<std::string>()) ? type_it->second.get<std::string>()
                                                                   : std::string();
    auto get = [&](const char* key) -> std::string {
        auto it = obj.find(key);
        if (it == obj.end()) {
            return {};
        }
        if (it->second.is<std::string>()) {
            return it->second.get<std::string>();
        }
        if (it->second.is<double>()) {
            std::ostringstream o;
            o << static_cast<unsigned long long>(it->second.get<double>());
            return o.str();
        }
        return json_min(it->second);
    };

    if (type == "privkey" || type == "pubkey" || type == "address") {
        return get("data");
    }
    if (type == "balance" || type == "inflow") {
        return get("sats");
    }
    if (type == "version") {
        return get("version");
    }
    if (type == "node") {
        const std::string ip = get("ip");
        const std::string port = get("port");
        if (!ip.empty() && !port.empty()) {
            return ip + ":" + port;
        }
        return get("host");
    }
    if (type == "config") {
        const bool has_dotted = obj.find("rpc.host") != obj.end() ||
                                obj.find("rpc.port") != obj.end() ||
                                obj.find("rpc.auth") != obj.end();
        if (!has_dotted) {
            return get("data");
        }
        static const char* kOrder[] = {"rpc.host", "rpc.port", "rpc.auth"};
        std::ostringstream o;
        bool first = true;
        for (const char* key : kOrder) {
            auto it = obj.find(key);
            if (it == obj.end()) {
                continue;
            }
            if (!first) {
                o << '\n';
            }
            first = false;
            o << key << '=';
            if (it->second.is<std::string>()) {
                o << it->second.get<std::string>();
            } else if (it->second.is<double>()) {
                o << static_cast<unsigned long long>(it->second.get<double>());
            } else {
                o << json_min(it->second);
            }
        }
        return o.str();
    }
    auto data = obj.find("data");
    if (data != obj.end() && data->second.is<std::string>()) {
        return data->second.get<std::string>();
    }
    return json_min(JsonValue(obj));
}

JsonObject version_object() {
    JsonObject o;
    set_string(o, "type", "version");
    set_string(o, "version", BTK_VERSION_STRING);
    set_bool(o, "secp256k1", true);
#ifdef BTK_NO_LEVELDB
    set_bool(o, "leveldb", false);
#else
    set_bool(o, "leveldb", true);
#endif
    return o;
}

OutputWriter::OutputWriter(const Options& opts) : opts_(opts) {}

OutputWriter::~OutputWriter() {
    if (!finished_) {
        try {
            finish();
        } catch (...) {
        }
    }
}

void OutputWriter::write(const JsonObject& obj) {
    const bool as_ndjson = opts_.out == OutFormat::Ndjson ||
                           (opts_.out == OutFormat::Json && opts_.stream);
    if (opts_.out == OutFormat::Plain) {
        std::cout << primary_string(obj) << '\n';
        std::fflush(stdout);
        return;
    }
    if (as_ndjson) {
        std::cout << json_min(JsonValue(obj)) << '\n';
        std::fflush(stdout);
        return;
    }
    buffered_.push_back(JsonValue(obj));
}

void OutputWriter::finish() {
    finished_ = true;
    if (opts_.out != OutFormat::Json || opts_.stream) {
        return;
    }
    if (buffered_.empty()) {
        return;
    }
    if (buffered_.size() == 1) {
        std::cout << json_pretty(buffered_[0]) << '\n';
    } else {
        std::cout << json_pretty(JsonValue(buffered_)) << '\n';
    }
}
