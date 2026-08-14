#include "cli/io.hpp"

#include "util/error.hpp"

#include <cctype>
#include <iostream>
#include <sstream>

JsonObject item_from_text(const std::string& text) {
    std::size_t i = 0;
    while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) {
        ++i;
    }
    if (i < text.size() && text[i] == '{') {
        return parse_json_object(text);
    }
    return make_bare(text);
}

namespace {

void walk_json_value(const JsonValue& v, const ItemHandler& handler, const std::string& command) {
    if (v.is<JsonArray>()) {
        for (const JsonValue& el : v.get<JsonArray>()) {
            if (el.is<JsonObject>()) {
                handler(el.get<JsonObject>());
            } else if (el.is<std::string>()) {
                handler(item_from_text(el.get<std::string>()));
            } else {
                throw BtkError(command, "expected a JSON object or string");
            }
        }
        return;
    }
    if (v.is<JsonObject>()) {
        handler(v.get<JsonObject>());
        return;
    }
    if (v.is<std::string>()) {
        handler(item_from_text(v.get<std::string>()));
        return;
    }
    throw BtkError(command, "expected a JSON object or string");
}

void for_each_line(std::istream& in, bool as_json_line, const ItemHandler& handler) {
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (as_json_line) {
            handler(parse_json_object(line));
        } else {
            handler(item_from_text(line));
        }
    }
}

}  // namespace

void for_each_stdin_item(const Options& opts, const ItemHandler& handler) {
    std::istream& in = std::cin;

    if (opts.in == InFormat::Json) {
        std::ostringstream buf;
        buf << in.rdbuf();
        const std::string text = buf.str();
        if (text.find_first_not_of(" \t\r\n") == std::string::npos) {
            return;
        }
        walk_json_value(parse_json_value(text, opts.command), handler, opts.command);
        return;
    }

    if (opts.in == InFormat::Ndjson) {
        for_each_line(in, true, handler);
        return;
    }

    if (opts.in == InFormat::Plain) {
        for_each_line(in, false, handler);
        return;
    }

    // auto
    int c = in.peek();
    while (c != EOF && std::isspace(static_cast<unsigned char>(c))) {
        in.get();
        c = in.peek();
    }
    if (c == EOF) {
        return;
    }
    if (c == '[') {
        std::string text;
        if (!extract_json_value(in, text)) {
            return;
        }
        walk_json_value(parse_json_value(text, opts.command), handler, opts.command);
        return;
    }
    if (c == '{') {
        std::string text;
        while (extract_json_value(in, text)) {
            handler(parse_json_object(text, opts.command));
            c = in.peek();
            while (c != EOF && std::isspace(static_cast<unsigned char>(c))) {
                in.get();
                c = in.peek();
            }
            if (c != '{') {
                break;
            }
        }
        return;
    }
    for_each_line(in, false, handler);
}
