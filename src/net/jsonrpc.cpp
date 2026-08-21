#include "net/jsonrpc.hpp"

#include "core/hex.hpp"
#include "util/error.hpp"
#include "util/interrupt.hpp"

#include <cmath>
#include <cstdint>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <sstream>

namespace {

[[noreturn]] void fail(const std::string& command, const std::string& message) {
    throw BtkError(command, message);
}

std::string base64_encode(const std::string& in) {
    static const char kTab[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);
    std::size_t i = 0;
    while (i + 3 <= in.size()) {
        const unsigned n = (static_cast<unsigned char>(in[i]) << 16) |
                           (static_cast<unsigned char>(in[i + 1]) << 8) |
                           static_cast<unsigned char>(in[i + 2]);
        out.push_back(kTab[(n >> 18) & 63]);
        out.push_back(kTab[(n >> 12) & 63]);
        out.push_back(kTab[(n >> 6) & 63]);
        out.push_back(kTab[n & 63]);
        i += 3;
    }
    if (i < in.size()) {
        unsigned n = static_cast<unsigned char>(in[i]) << 16;
        if (i + 1 < in.size()) {
            n |= static_cast<unsigned char>(in[i + 1]) << 8;
        }
        out.push_back(kTab[(n >> 18) & 63]);
        out.push_back(kTab[(n >> 12) & 63]);
        out.push_back(i + 1 < in.size() ? kTab[(n >> 6) & 63] : '=');
        out.push_back('=');
    }
    return out;
}

class Fd {
public:
    explicit Fd(int fd = -1) : fd_(fd) {}
    ~Fd() { close(); }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }
    int get() const { return fd_; }
    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

private:
    int fd_;
};

void wait_fd(int fd, short events, const std::string& command, int timeout_ms) {
    int left = timeout_ms;
    while (left > 0) {
        if (stop_requested()) {
            fail(command, "interrupted");
        }
        const int slice = left < 200 ? left : 200;
        pollfd p{};
        p.fd = fd;
        p.events = events;
        const int r = poll(&p, 1, slice);
        if (r > 0) {
            return;
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            fail(command, "rpc poll failed");
        }
        left -= slice;
    }
    fail(command, "rpc timeout");
}

void write_all(int fd, const std::string& s, const std::string& command, int timeout_ms) {
    std::size_t off = 0;
    while (off < s.size()) {
        wait_fd(fd, POLLOUT, command, timeout_ms);
        const ssize_t n = ::write(fd, s.data() + off, s.size() - off);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            fail(command, "rpc write failed");
        }
        off += static_cast<std::size_t>(n);
    }
}

std::string read_all(int fd, const std::string& command, int timeout_ms) {
    std::string out;
    char buf[4096];
    while (true) {
        wait_fd(fd, POLLIN, command, timeout_ms);
        const ssize_t n = ::read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            fail(command, "rpc read failed");
        }
        if (n == 0) {
            break;
        }
        out.append(buf, static_cast<std::size_t>(n));
    }
    return out;
}

int connect_ipv4(const std::string& host, std::uint16_t port, const std::string& command,
                 int timeout_ms) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = nullptr;
    const int g = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (g != 0 || res == nullptr) {
        fail(command, "cannot connect to rpc");
    }
    int fd = -1;
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) {
            continue;
        }
        const int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        const int c = connect(fd, p->ai_addr, p->ai_addrlen);
        if (c == 0 || errno == EINPROGRESS) {
            pollfd pol{};
            pol.fd = fd;
            pol.events = POLLOUT;
            if (poll(&pol, 1, timeout_ms) > 0) {
                int err = 0;
                socklen_t elen = sizeof(err);
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &elen) == 0 && err == 0) {
                    break;
                }
            }
        }
        ::close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) {
        fail(command, "cannot connect to rpc");
    }
    return fd;
}

std::string http_body(const std::string& resp, const std::string& command) {
    const auto pos = resp.find("\r\n\r\n");
    if (pos == std::string::npos) {
        fail(command, "invalid rpc response");
    }
    const std::string headers = resp.substr(0, pos);
    if (headers.find(" 200 ") == std::string::npos && headers.compare(0, 12, "HTTP/1.0 200") != 0 &&
        headers.compare(0, 12, "HTTP/1.1 200") != 0) {
        if (headers.find(" 401 ") != std::string::npos) {
            fail(command, "rpc authentication failed");
        }
        fail(command, "rpc request failed");
    }
    return resp.substr(pos + 4);
}

}  // namespace

JsonRpc::JsonRpc(std::string host, std::uint16_t port, std::string auth, std::string command)
    : host_(std::move(host)), port_(port), auth_(std::move(auth)), command_(std::move(command)) {}

JsonValue JsonRpc::call(const std::string& method, const JsonArray& params) {
    JsonObject req;
    set_string(req, "jsonrpc", "1.0");
    set_string(req, "id", "btk");
    set_string(req, "method", method);
    req["params"] = JsonValue(params);
    const std::string body = json_min(JsonValue(req));

    std::ostringstream hdr;
    hdr << "POST / HTTP/1.0\r\n";
    hdr << "Host: " << host_ << ':' << port_ << "\r\n";
    hdr << "Content-Type: application/json\r\n";
    if (!auth_.empty()) {
        hdr << "Authorization: Basic " << base64_encode(auth_) << "\r\n";
    }
    hdr << "Content-Length: " << body.size() << "\r\n";
    hdr << "Connection: close\r\n\r\n";
    const std::string wire = hdr.str() + body;

    Fd fd(connect_ipv4(host_, port_, command_, timeout_ms_));
    write_all(fd.get(), wire, command_, timeout_ms_);
    const std::string resp = read_all(fd.get(), command_, timeout_ms_);
    const JsonValue parsed = parse_json_value(http_body(resp, command_), command_);
    if (!parsed.is<JsonObject>()) {
        fail(command_, "invalid rpc response");
    }
    const JsonObject& obj = parsed.get<JsonObject>();
    auto err = obj.find("error");
    if (err != obj.end() && !err->second.is<picojson::null>()) {
        fail(command_, "rpc request failed");
    }
    auto result = obj.find("result");
    if (result == obj.end()) {
        fail(command_, "invalid rpc response");
    }
    return result->second;
}

std::uint32_t rpc_getblockcount(JsonRpc& rpc) {
    const JsonValue v = rpc.call("getblockcount");
    if (!v.is<double>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    const double d = v.get<double>();
    if (d < 0 || d > 4294967295.0 || d != static_cast<double>(static_cast<std::uint32_t>(d))) {
        fail(rpc.command(), "invalid rpc response");
    }
    return static_cast<std::uint32_t>(d);
}

std::string rpc_getblockhash(JsonRpc& rpc, std::uint32_t height) {
    JsonArray params;
    params.emplace_back(static_cast<double>(height));
    const JsonValue v = rpc.call("getblockhash", params);
    if (!v.is<std::string>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    return v.get<std::string>();
}

std::vector<std::uint8_t> rpc_getblock(JsonRpc& rpc, const std::string& hash_hex) {
    JsonArray params;
    params.emplace_back(hash_hex);
    params.emplace_back(0.0);
    const JsonValue v = rpc.call("getblock", params);
    if (!v.is<std::string>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    try {
        return hex_decode(v.get<std::string>());
    } catch (const BtkError&) {
        fail(rpc.command(), "invalid rpc response");
    }
}

namespace {

bool as_uint32(const JsonValue& v, std::uint32_t& out) {
    if (!v.is<double>()) {
        return false;
    }
    const double d = v.get<double>();
    if (d < 0 || d > 4294967295.0 || d != static_cast<double>(static_cast<std::uint32_t>(d))) {
        return false;
    }
    out = static_cast<std::uint32_t>(d);
    return true;
}

bool btc_to_sats(double btc, std::uint64_t& out) {
    if (!std::isfinite(btc) || btc < 0 || btc > 21000000.0) {
        return false;
    }
    const double s = std::llround(btc * 100000000.0);
    if (s < 0 || s > static_cast<double>(UINT64_MAX)) {
        return false;
    }
    out = static_cast<std::uint64_t>(s);
    return true;
}

}  // namespace

ScanUtxoSet rpc_scantxoutset(JsonRpc& rpc, const std::string& address) {
    JsonArray descriptors;
    descriptors.emplace_back("addr(" + address + ")");
    JsonArray params;
    params.emplace_back("start");
    params.emplace_back(descriptors);
    const JsonValue v = rpc.call("scantxoutset", params);
    if (!v.is<JsonObject>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    const JsonObject& obj = v.get<JsonObject>();
    ScanUtxoSet out;
    auto success = obj.find("success");
    if (success == obj.end() || !success->second.is<bool>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    out.success = success->second.get<bool>();
    auto height = obj.find("height");
    if (height == obj.end() || !as_uint32(height->second, out.height)) {
        fail(rpc.command(), "invalid rpc response");
    }
    auto uns = obj.find("unspents");
    if (uns == obj.end() || !uns->second.is<JsonArray>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    for (const JsonValue& item : uns->second.get<JsonArray>()) {
        if (!item.is<JsonObject>()) {
            fail(rpc.command(), "invalid rpc response");
        }
        const JsonObject& u = item.get<JsonObject>();
        RpcUtxo ru;
        auto txid = u.find("txid");
        auto vout = u.find("vout");
        auto spk = u.find("scriptPubKey");
        auto amount = u.find("amount");
        if (txid == u.end() || !txid->second.is<std::string>() || vout == u.end() ||
            !as_uint32(vout->second, ru.vout) || spk == u.end() || !spk->second.is<std::string>() ||
            amount == u.end() || !amount->second.is<double>()) {
            fail(rpc.command(), "invalid rpc response");
        }
        ru.txid = txid->second.get<std::string>();
        ru.script_hex = spk->second.get<std::string>();
        if (!btc_to_sats(amount->second.get<double>(), ru.amount_sats)) {
            fail(rpc.command(), "invalid rpc response");
        }
        auto cb = u.find("coinbase");
        if (cb != u.end() && cb->second.is<bool>()) {
            ru.coinbase = cb->second.get<bool>();
        }
        auto uh = u.find("height");
        if (uh != u.end() && !as_uint32(uh->second, ru.height)) {
            fail(rpc.command(), "invalid rpc response");
        }
        out.unspents.push_back(std::move(ru));
    }
    return out;
}

std::optional<std::uint64_t> rpc_estimatesmartfee_satvb(JsonRpc& rpc, int conf_target) {
    JsonArray params;
    params.emplace_back(static_cast<double>(conf_target));
    const JsonValue v = rpc.call("estimatesmartfee", params);
    if (!v.is<JsonObject>()) {
        fail(rpc.command(), "invalid rpc response");
    }
    const JsonObject& obj = v.get<JsonObject>();
    auto fr = obj.find("feerate");
    if (fr == obj.end() || !fr->second.is<double>()) {
        return std::nullopt;
    }
    const double btc_kvb = fr->second.get<double>();
    if (!std::isfinite(btc_kvb) || btc_kvb <= 0) {
        return std::nullopt;
    }
    // BTC/kvB → sat/vB: * 1e8 / 1000 = * 1e5
    const double satvb = std::llround(btc_kvb * 100000.0);
    if (satvb < 1) {
        return 1;
    }
    return static_cast<std::uint64_t>(satvb);
}

std::string rpc_sendrawtransaction(JsonRpc& rpc, const std::string& hex) {
    JsonArray params;
    params.emplace_back(hex);
    const JsonValue v = rpc.call("sendrawtransaction", params);
    if (!v.is<std::string>() || v.get<std::string>().empty()) {
        fail(rpc.command(), "invalid rpc response");
    }
    return v.get<std::string>();
}
