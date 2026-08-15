#include "net/p2p.hpp"

#include "core/hash.hpp"
#include "util/error.hpp"
#include "version.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <ctime>
#include <cstring>
#include <utility>

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw BtkError("node", message);
}

void put_u8(std::vector<std::uint8_t>& o, std::uint8_t v) {
    o.push_back(v);
}

void put_le16(std::vector<std::uint8_t>& o, std::uint16_t v) {
    o.push_back(static_cast<std::uint8_t>(v));
    o.push_back(static_cast<std::uint8_t>(v >> 8));
}

void put_be16(std::vector<std::uint8_t>& o, std::uint16_t v) {
    o.push_back(static_cast<std::uint8_t>(v >> 8));
    o.push_back(static_cast<std::uint8_t>(v));
}

void put_le32(std::vector<std::uint8_t>& o, std::uint32_t v) {
    o.push_back(static_cast<std::uint8_t>(v));
    o.push_back(static_cast<std::uint8_t>(v >> 8));
    o.push_back(static_cast<std::uint8_t>(v >> 16));
    o.push_back(static_cast<std::uint8_t>(v >> 24));
}

void put_le64(std::vector<std::uint8_t>& o, std::uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        o.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
    }
}

void put_bytes(std::vector<std::uint8_t>& o, const std::uint8_t* p, std::size_t n) {
    o.insert(o.end(), p, p + n);
}

std::uint16_t get_le16(const std::uint8_t* p) {
    return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1]) << 8));
}

std::uint16_t get_be16(const std::uint8_t* p) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(p[0]) << 8) | p[1]);
}

std::uint32_t get_le32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}

std::uint64_t get_le64(const std::uint8_t* p) {
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v |= static_cast<std::uint64_t>(p[i]) << (8 * i);
    }
    return v;
}

void put_compact_size(std::vector<std::uint8_t>& o, std::uint64_t n) {
    if (n < 0xfd) {
        put_u8(o, static_cast<std::uint8_t>(n));
    } else if (n <= 0xffff) {
        put_u8(o, 0xfd);
        put_le16(o, static_cast<std::uint16_t>(n));
    } else if (n <= 0xffffffffu) {
        put_u8(o, 0xfe);
        put_le32(o, static_cast<std::uint32_t>(n));
    } else {
        put_u8(o, 0xff);
        put_le64(o, n);
    }
}

std::uint64_t get_compact_size(const std::uint8_t* data, std::size_t len, std::size_t& off) {
    if (off >= len) {
        fail("invalid version message");
    }
    const std::uint8_t b = data[off++];
    if (b < 0xfd) {
        return b;
    }
    if (b == 0xfd) {
        if (off + 2 > len) {
            fail("invalid version message");
        }
        const std::uint64_t n = get_le16(data + off);
        off += 2;
        if (n < 0xfd) {
            fail("invalid version message");
        }
        return n;
    }
    if (b == 0xfe) {
        if (off + 4 > len) {
            fail("invalid version message");
        }
        const std::uint64_t n = get_le32(data + off);
        off += 4;
        if (n <= 0xffff) {
            fail("invalid version message");
        }
        return n;
    }
    if (off + 8 > len) {
        fail("invalid version message");
    }
    const std::uint64_t n = get_le64(data + off);
    off += 8;
    if (n <= 0xffffffffu) {
        fail("invalid version message");
    }
    return n;
}

void put_net_addr(std::vector<std::uint8_t>& o, const NetAddr& addr) {
    put_le64(o, addr.services);
    put_bytes(o, addr.ip.data(), addr.ip.size());
    put_be16(o, addr.port);
}

NetAddr get_net_addr(const std::uint8_t* data, std::size_t len, std::size_t& off) {
    if (off + 26 > len) {
        fail("invalid version message");
    }
    NetAddr addr;
    addr.services = get_le64(data + off);
    off += 8;
    std::memcpy(addr.ip.data(), data + off, 16);
    off += 16;
    addr.port = get_be16(data + off);
    off += 2;
    return addr;
}

std::array<std::uint8_t, 4> payload_checksum(const std::vector<std::uint8_t>& payload) {
    const Hash256 h = hash256(payload);
    return {{h[0], h[1], h[2], h[3]}};
}

std::string command_from_header(const std::uint8_t* cmd) {
    std::size_t n = 0;
    while (n < 12 && cmd[n] != 0) {
        ++n;
    }
    for (std::size_t i = n; i < 12; ++i) {
        if (cmd[i] != 0) {
            fail("invalid P2P command");
        }
    }
    return std::string(reinterpret_cast<const char*>(cmd), n);
}

void pad_command(std::uint8_t out[12], const std::string& command) {
    if (command.size() > 12) {
        fail("invalid P2P command");
    }
    std::memset(out, 0, 12);
    std::memcpy(out, command.data(), command.size());
}

bool is_v4_mapped(const std::array<std::uint8_t, 16>& ip) {
    for (int i = 0; i < 10; ++i) {
        if (ip[static_cast<std::size_t>(i)] != 0) {
            return false;
        }
    }
    return ip[10] == 0xff && ip[11] == 0xff;
}

class Fd {
public:
    explicit Fd(int fd) : fd_(fd) {}
    ~Fd() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }
    int get() const { return fd_; }

private:
    int fd_;
};

void set_timeouts(int fd) {
    timeval tv{};
    tv.tv_sec = kP2pTimeoutSec;
    tv.tv_usec = 0;
    if (::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0 ||
        ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) {
        fail("connect failed");
    }
}

bool timed_out(int err) {
    return err == EAGAIN || err == EWOULDBLOCK || err == ETIMEDOUT;
}

void write_exact(int fd, const std::uint8_t* data, std::size_t n) {
    std::size_t off = 0;
    while (off < n) {
        const ssize_t w = ::send(fd, data + off, n - off, MSG_NOSIGNAL);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (timed_out(errno)) {
                fail("timed out after 15s");
            }
            fail("write failed");
        }
        if (w == 0) {
            fail("peer closed connection");
        }
        off += static_cast<std::size_t>(w);
    }
}

void read_exact(int fd, std::uint8_t* data, std::size_t n) {
    std::size_t off = 0;
    while (off < n) {
        const ssize_t r = ::recv(fd, data + off, n - off, 0);
        if (r == 0) {
            fail("peer closed connection");
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (timed_out(errno)) {
                fail("timed out after 15s");
            }
            fail("read failed");
        }
        off += static_cast<std::size_t>(r);
    }
}

int connect_one(const addrinfo* ai, bool& timed_out_flag, int& err_out) {
    timed_out_flag = false;
    err_out = 0;
#ifdef SOCK_CLOEXEC
    const int fd = ::socket(ai->ai_family, ai->ai_socktype | SOCK_CLOEXEC, ai->ai_protocol);
#else
    const int fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
#endif
    if (fd < 0) {
        err_out = errno;
        return -1;
    }
#ifndef SOCK_CLOEXEC
    ::fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif

    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
        err_out = errno;
        ::close(fd);
        return -1;
    }

    int rc = ::connect(fd, ai->ai_addr, ai->ai_addrlen);
    if (rc != 0 && errno != EINPROGRESS) {
        err_out = errno;
        ::close(fd);
        if (err_out == ETIMEDOUT) {
            timed_out_flag = true;
        }
        return -1;
    }

    if (rc != 0) {
        pollfd pfd{};
        pfd.fd = fd;
        pfd.events = POLLOUT;
        const int pr = ::poll(&pfd, 1, kP2pTimeoutSec * 1000);
        if (pr == 0) {
            ::close(fd);
            timed_out_flag = true;
            err_out = ETIMEDOUT;
            return -1;
        }
        if (pr < 0) {
            err_out = errno;
            ::close(fd);
            return -1;
        }
        int err = 0;
        socklen_t len = sizeof(err);
        if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) != 0 || err != 0) {
            err_out = err != 0 ? err : errno;
            if (err_out == ETIMEDOUT) {
                timed_out_flag = true;
            }
            ::close(fd);
            return -1;
        }
    }

    if (::fcntl(fd, F_SETFL, flags) != 0) {
        err_out = errno;
        ::close(fd);
        return -1;
    }

    const int yes = 1;
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    set_timeouts(fd);
    return fd;
}

std::string sockaddr_ipv4(const sockaddr* sa) {
    if (sa == nullptr || sa->sa_family != AF_INET) {
        return {};
    }
    char buf[INET_ADDRSTRLEN] = {};
    const auto* in = reinterpret_cast<const sockaddr_in*>(sa);
    if (::inet_ntop(AF_INET, &in->sin_addr, buf, sizeof(buf)) == nullptr) {
        return {};
    }
    return buf;
}

}  // namespace

std::string default_user_agent() {
    return std::string("/Bitcoin-Toolkit:") + BTK_VERSION_STRING + "/";
}

std::array<std::uint8_t, 16> ipv4_mapped(std::uint8_t a, std::uint8_t b, std::uint8_t c,
                                         std::uint8_t d) {
    std::array<std::uint8_t, 16> ip{};
    ip[10] = 0xff;
    ip[11] = 0xff;
    ip[12] = a;
    ip[13] = b;
    ip[14] = c;
    ip[15] = d;
    return ip;
}

std::string format_net_ip(const std::array<std::uint8_t, 16>& ip) {
    if (is_v4_mapped(ip)) {
        char buf[INET_ADDRSTRLEN] = {};
        if (::inet_ntop(AF_INET, ip.data() + 12, buf, sizeof(buf)) == nullptr) {
            return {};
        }
        return buf;
    }
    char buf[INET6_ADDRSTRLEN] = {};
    if (::inet_ntop(AF_INET6, ip.data(), buf, sizeof(buf)) == nullptr) {
        return {};
    }
    return buf;
}

std::vector<std::string> service_names(std::uint64_t services) {
    static const char* kNamed[] = {
        "NODE_NETWORK",          // 0
        "NODE_GETUTXO",          // 1
        "NODE_BLOOM",            // 2
        "NODE_WITNESS",          // 3
        "NODE_XTHIN",            // 4
        nullptr,                 // 5
        "NODE_COMPACT_FILTERS",  // 6
        nullptr,                 // 7
        nullptr,                 // 8
        nullptr,                 // 9
        "NODE_NETWORK_LIMITED",  // 10
    };

    std::vector<std::string> names;
    for (int i = 0; i < 64; ++i) {
        if ((services & (1ull << i)) == 0) {
            continue;
        }
        if (i < static_cast<int>(sizeof(kNamed) / sizeof(kNamed[0])) && kNamed[i] != nullptr) {
            names.emplace_back(kNamed[i]);
        } else {
            names.push_back("BIT_" + std::to_string(i));
        }
    }
    return names;
}

VersionPayload make_outbound_version(std::int64_t timestamp) {
    VersionPayload msg;
    msg.version = kProtocolVersion;
    msg.services = 0;
    msg.timestamp = timestamp;
    msg.addr_recv.services = 0;
    msg.addr_recv.ip = ipv4_mapped(127, 0, 0, 1);
    msg.addr_recv.port = kMainnetPort;
    msg.addr_trans = msg.addr_recv;
    msg.nonce = 0;
    msg.user_agent = default_user_agent();
    msg.start_height = 0;
    msg.relay = false;
    return msg;
}

std::vector<std::uint8_t> serialize_version_payload(const VersionPayload& msg) {
    std::vector<std::uint8_t> o;
    o.reserve(109);
    put_le32(o, static_cast<std::uint32_t>(msg.version));
    put_le64(o, msg.services);
    put_le64(o, static_cast<std::uint64_t>(msg.timestamp));
    put_net_addr(o, msg.addr_recv);
    put_net_addr(o, msg.addr_trans);
    put_le64(o, msg.nonce);
    put_compact_size(o, msg.user_agent.size());
    put_bytes(o, reinterpret_cast<const std::uint8_t*>(msg.user_agent.data()),
              msg.user_agent.size());
    put_le32(o, static_cast<std::uint32_t>(msg.start_height));
    put_u8(o, msg.relay ? 1 : 0);
    return o;
}

VersionPayload parse_version_payload(const std::uint8_t* data, std::size_t len) {
    // version + services + timestamp + 2×net_addr + nonce + CompactSize + height
    if (data == nullptr || len < 80 + 1 + 4) {
        fail("invalid version message");
    }
    std::size_t off = 0;
    VersionPayload msg;
    msg.version = static_cast<std::int32_t>(get_le32(data + off));
    off += 4;
    msg.services = get_le64(data + off);
    off += 8;
    msg.timestamp = static_cast<std::int64_t>(get_le64(data + off));
    off += 8;
    msg.addr_recv = get_net_addr(data, len, off);
    msg.addr_trans = get_net_addr(data, len, off);
    if (off + 8 > len) {
        fail("invalid version message");
    }
    msg.nonce = get_le64(data + off);
    off += 8;

    const std::uint64_t ua_len = get_compact_size(data, len, off);
    if (ua_len > 256 || off + ua_len > len) {
        fail("invalid version message");
    }
    msg.user_agent.assign(reinterpret_cast<const char*>(data + off),
                          static_cast<std::size_t>(ua_len));
    off += static_cast<std::size_t>(ua_len);

    if (off + 4 > len) {
        fail("invalid version message");
    }
    msg.start_height = static_cast<std::int32_t>(get_le32(data + off));
    off += 4;
    if (off < len) {
        msg.relay = data[off] != 0;
    }
    return msg;
}

VersionPayload parse_version_payload(const std::vector<std::uint8_t>& data) {
    return parse_version_payload(data.data(), data.size());
}

std::vector<std::uint8_t> serialize_p2p_message(const std::string& command,
                                                const std::vector<std::uint8_t>& payload) {
    std::vector<std::uint8_t> o;
    o.reserve(24 + payload.size());
    put_bytes(o, kMainnetMagic.data(), kMainnetMagic.size());
    std::uint8_t cmd[12];
    pad_command(cmd, command);
    put_bytes(o, cmd, 12);
    put_le32(o, static_cast<std::uint32_t>(payload.size()));
    const auto sum = payload_checksum(payload);
    put_bytes(o, sum.data(), sum.size());
    put_bytes(o, payload.data(), payload.size());
    return o;
}

P2pMessage parse_p2p_message(const std::uint8_t* data, std::size_t len) {
    if (data == nullptr || len < 24) {
        fail("invalid version message");
    }
    if (std::memcmp(data, kMainnetMagic.data(), 4) != 0) {
        fail("invalid P2P magic");
    }
    P2pMessage msg;
    msg.command = command_from_header(data + 4);
    const std::uint32_t payload_len = get_le32(data + 16);
    if (len != 24 + payload_len) {
        fail("invalid version message");
    }
    const std::uint8_t* got = data + 20;
    msg.payload.assign(data + 24, data + 24 + payload_len);
    const auto want = payload_checksum(msg.payload);
    if (std::memcmp(got, want.data(), 4) != 0) {
        fail("invalid version checksum");
    }
    return msg;
}

P2pMessage parse_p2p_message(const std::vector<std::uint8_t>& data) {
    return parse_p2p_message(data.data(), data.size());
}

NodePeer handshake_version(const std::string& host, std::uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* raw = nullptr;
    const int gerr = ::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &raw);
    if (gerr != 0 || raw == nullptr) {
        if (raw != nullptr) {
            ::freeaddrinfo(raw);
        }
        fail("could not resolve host");
    }

    bool any_timeout = false;
    int last_errno = 0;
    int connected = -1;
    std::string ip;
    for (addrinfo* ai = raw; ai != nullptr; ai = ai->ai_next) {
        bool this_timeout = false;
        int this_err = 0;
        const int fd = connect_one(ai, this_timeout, this_err);
        if (fd >= 0) {
            connected = fd;
            ip = sockaddr_ipv4(ai->ai_addr);
            break;
        }
        if (this_timeout) {
            any_timeout = true;
        } else {
            last_errno = this_err;
        }
    }
    ::freeaddrinfo(raw);

    if (connected < 0) {
        if (any_timeout) {
            fail("connect timed out after 15s");
        }
        if (last_errno == ECONNREFUSED) {
            fail("connection refused");
        }
        fail("connect failed");
    }

    Fd sock(connected);
    if (ip.empty()) {
        sockaddr_in peer{};
        socklen_t plen = sizeof(peer);
        if (::getpeername(sock.get(), reinterpret_cast<sockaddr*>(&peer), &plen) == 0) {
            ip = sockaddr_ipv4(reinterpret_cast<sockaddr*>(&peer));
        }
    }

    const std::int64_t now = static_cast<std::int64_t>(::time(nullptr));
    const auto payload = serialize_version_payload(make_outbound_version(now));
    const auto wire = serialize_p2p_message("version", payload);
    write_exact(sock.get(), wire.data(), wire.size());

    std::uint8_t header[24];
    read_exact(sock.get(), header, 24);
    if (std::memcmp(header, kMainnetMagic.data(), 4) != 0) {
        fail("invalid P2P magic");
    }
    const std::string command = command_from_header(header + 4);
    if (command != "version") {
        fail("expected version message");
    }
    const std::uint32_t payload_len = get_le32(header + 16);
    if (payload_len < 85 || payload_len > 1024) {
        fail("invalid version message");
    }
    std::vector<std::uint8_t> peer_payload(payload_len);
    read_exact(sock.get(), peer_payload.data(), payload_len);
    const auto want = payload_checksum(peer_payload);
    if (std::memcmp(header + 20, want.data(), 4) != 0) {
        fail("invalid version checksum");
    }

    NodePeer peer;
    peer.host = host;
    peer.ip = ip;
    peer.port = port;
    peer.version = parse_version_payload(peer_payload);
    return peer;
}
