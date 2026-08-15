#include "cli/io.hpp"

#include "util/error.hpp"

#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unistd.h>

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

bool looks_like_binary(const std::string& buf) {
    const auto* p = reinterpret_cast<const unsigned char*>(buf.data());
    const std::size_t n = buf.size();
    for (std::size_t i = 0; i < n;) {
        const unsigned char c = p[i];
        if (c == 0) {
            return true;
        }
        if (c == '\t' || c == '\n' || c == '\r') {
            ++i;
            continue;
        }
        if (c < 0x20 || c == 0x7f) {
            return true;
        }
        if (c < 0x80) {
            ++i;
            continue;
        }
        int need = 0;
        if ((c & 0xe0) == 0xc0) {
            need = 1;
        } else if ((c & 0xf0) == 0xe0) {
            need = 2;
        } else if ((c & 0xf8) == 0xf0) {
            need = 3;
        } else {
            return true;
        }
        if (c < 0xc2 || c > 0xf4) {
            return true;
        }
        if (i + 1 + static_cast<std::size_t>(need) > n) {
            return false;  // truncated at prefix end; do not guess binary
        }
        for (int k = 1; k <= need; ++k) {
            if ((p[i + static_cast<std::size_t>(k)] & 0xc0) != 0x80) {
                return true;
            }
        }
        i += 1 + static_cast<std::size_t>(need);
    }
    return false;
}

std::string read_n(std::istream& in, std::size_t n) {
    std::string s;
    s.resize(n);
    in.read(s.data(), static_cast<std::streamsize>(n));
    s.resize(static_cast<std::size_t>(in.gcount()));
    return s;
}

std::string read_all(std::istream& in) {
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

class PrefixBuf : public std::streambuf {
public:
    PrefixBuf(std::string prefix, std::streambuf* rest) : prefix_(std::move(prefix)), rest_(rest) {
        if (!prefix_.empty()) {
            char* p = prefix_.data();
            setg(p, p, p + prefix_.size());
        }
    }

protected:
    int_type underflow() override {
        if (gptr() < egptr()) {
            return traits_type::to_int_type(*gptr());
        }
        if (rest_ == nullptr) {
            return traits_type::eof();
        }
        buf_.resize(4096);
        const std::streamsize n = rest_->sgetn(buf_.data(), static_cast<std::streamsize>(buf_.size()));
        if (n <= 0) {
            return traits_type::eof();
        }
        buf_.resize(static_cast<std::size_t>(n));
        setg(buf_.data(), buf_.data(), buf_.data() + buf_.size());
        return traits_type::to_int_type(*gptr());
    }

private:
    std::string prefix_;
    std::string buf_;
    std::streambuf* rest_;
};

class PrefixStream : public std::istream {
public:
    PrefixStream(std::string prefix, std::istream& rest)
        : std::istream(nullptr), buf_(std::move(prefix), rest.rdbuf()) {
        rdbuf(&buf_);
    }

private:
    PrefixBuf buf_;
};

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

void for_each_auto(std::istream& in, const Options& opts, const ItemHandler& handler) {
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

    // auto: on a pipe, look at a prefix. Binary → one raw item (privkey hashes it).
    // A TTY stays line-oriented so a typed WIF is not blocked on an 8KiB peek.
    if (!isatty(STDIN_FILENO)) {
        constexpr std::size_t kPrefix = 8192;
        std::string prefix = read_n(in, kPrefix);
        if (prefix.empty() && in.eof()) {
            return;
        }
        if (looks_like_binary(prefix)) {
            prefix += read_all(in);
            handler(make_bare(prefix));
            return;
        }
        PrefixStream wrapped(std::move(prefix), in);
        for_each_auto(wrapped, opts, handler);
        return;
    }

    for_each_auto(in, opts, handler);
}
