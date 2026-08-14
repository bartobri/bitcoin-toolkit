#include "core/json_io.hpp"

#include "util/error.hpp"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

JsonObject make_bare(const std::string& text) {
    JsonObject o;
    o[kBareField] = JsonValue(text);
    return o;
}

bool is_bare(const JsonObject& obj) {
    return obj.find(kBareField) != obj.end() && obj.find("type") == obj.end();
}

std::string bare_text(const JsonObject& obj) {
    auto it = obj.find(kBareField);
    if (it == obj.end() || !it->second.is<std::string>()) {
        throw BtkError("", "internal: missing bare item");
    }
    return it->second.get<std::string>();
}

JsonValue parse_json_value(const std::string& text, const std::string& command) {
    JsonValue v;
    const std::string err = picojson::parse(v, text);
    if (!err.empty()) {
        throw BtkError(command, "invalid JSON");
    }
    return v;
}

JsonObject parse_json_object(const std::string& text, const std::string& command) {
    JsonValue v = parse_json_value(text, command);
    if (!v.is<JsonObject>()) {
        throw BtkError(command, "expected a JSON object");
    }
    return v.get<JsonObject>();
}

bool extract_json_value(std::istream& in, std::string& text) {
    text.clear();
    int c = in.peek();
    while (c != EOF && std::isspace(static_cast<unsigned char>(c))) {
        in.get();
        c = in.peek();
    }
    if (c == EOF) {
        return false;
    }

    if (c != '{' && c != '[') {
        // Atom: string, number, true/false/null — read until whitespace or , ] }
        if (c == '"') {
            text.push_back(static_cast<char>(in.get()));
            bool esc = false;
            while (true) {
                c = in.get();
                if (c == EOF) {
                    throw BtkError("", "truncated JSON");
                }
                text.push_back(static_cast<char>(c));
                if (esc) {
                    esc = false;
                } else if (c == '\\') {
                    esc = true;
                } else if (c == '"') {
                    break;
                }
            }
            return true;
        }
        while (c != EOF && !std::isspace(static_cast<unsigned char>(c)) && c != ',' && c != ']' &&
               c != '}') {
            text.push_back(static_cast<char>(in.get()));
            c = in.peek();
        }
        if (text.empty()) {
            return false;
        }
        return true;
    }

    const char open = static_cast<char>(in.get());
    text.push_back(open);
    int depth = 1;
    bool in_str = false;
    bool esc = false;
    while (depth > 0) {
        c = in.get();
        if (c == EOF) {
            throw BtkError("", "truncated JSON");
        }
        text.push_back(static_cast<char>(c));
        if (in_str) {
            if (esc) {
                esc = false;
            } else if (c == '\\') {
                esc = true;
            } else if (c == '"') {
                in_str = false;
            }
            continue;
        }
        if (c == '"') {
            in_str = true;
        } else if (c == '{' || c == '[') {
            ++depth;
        } else if (c == '}' || c == ']') {
            --depth;
        }
    }
    return true;
}

namespace {

void emit(std::ostream& o, const JsonValue& v, int indent, int level);

void emit_indent(std::ostream& o, int n) {
    for (int i = 0; i < n; ++i) {
        o << ' ';
    }
}

void emit_number(std::ostream& o, double d) {
    if (std::isfinite(d) && d == std::floor(d) && d >= 0 && d < 9007199254740992.0) {
        o << static_cast<unsigned long long>(d);
        return;
    }
    o << d;
}

void emit(std::ostream& o, const JsonValue& v, int indent, int level) {
    if (v.is<picojson::null>()) {
        o << "null";
        return;
    }
    if (v.is<bool>()) {
        o << (v.get<bool>() ? "true" : "false");
        return;
    }
    if (v.is<double>()) {
        emit_number(o, v.get<double>());
        return;
    }
    if (v.is<std::string>()) {
        o << JsonValue(v.get<std::string>()).serialize();
        return;
    }
    if (v.is<JsonArray>()) {
        const JsonArray& a = v.get<JsonArray>();
        if (indent < 0) {
            o << '[';
            for (std::size_t i = 0; i < a.size(); ++i) {
                if (i) {
                    o << ',';
                }
                emit(o, a[i], indent, level);
            }
            o << ']';
            return;
        }
        o << "[\n";
        for (std::size_t i = 0; i < a.size(); ++i) {
            emit_indent(o, (level + 1) * indent);
            emit(o, a[i], indent, level + 1);
            if (i + 1 < a.size()) {
                o << ',';
            }
            o << '\n';
        }
        emit_indent(o, level * indent);
        o << ']';
        return;
    }
    if (v.is<JsonObject>()) {
        const JsonObject& obj = v.get<JsonObject>();
        if (indent < 0) {
            o << '{';
            bool first = true;
            for (const auto& kv : obj) {
                if (!first) {
                    o << ',';
                }
                first = false;
                o << JsonValue(kv.first).serialize() << ':';
                emit(o, kv.second, indent, level);
            }
            o << '}';
            return;
        }
        o << "{\n";
        std::size_t i = 0;
        for (const auto& kv : obj) {
            emit_indent(o, (level + 1) * indent);
            o << JsonValue(kv.first).serialize() << ": ";
            emit(o, kv.second, indent, level + 1);
            if (++i < obj.size()) {
                o << ',';
            }
            o << '\n';
        }
        emit_indent(o, level * indent);
        o << '}';
        return;
    }
}

}  // namespace

std::string json_min(const JsonValue& v) {
    std::ostringstream o;
    emit(o, v, -1, 0);
    return o.str();
}

std::string json_pretty(const JsonValue& v) {
    std::ostringstream o;
    emit(o, v, 2, 0);
    return o.str();
}

void set_string(JsonObject& o, const char* key, const std::string& value) {
    o[key] = JsonValue(value);
}

void set_bool(JsonObject& o, const char* key, bool value) {
    o[key] = JsonValue(value);
}

void set_uint64(JsonObject& o, const char* key, std::uint64_t value) {
    o[key] = JsonValue(static_cast<double>(value));
}
