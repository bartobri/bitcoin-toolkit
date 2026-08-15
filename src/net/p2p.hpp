#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

constexpr std::int32_t kProtocolVersion = 70015;
constexpr std::uint16_t kMainnetPort = 8333;
constexpr int kP2pTimeoutSec = 15;
constexpr std::array<std::uint8_t, 4> kMainnetMagic{{0xf9, 0xbe, 0xb4, 0xd9}};

struct NetAddr {
    std::uint64_t services = 0;
    std::array<std::uint8_t, 16> ip{};
    std::uint16_t port = 0;
};

struct VersionPayload {
    std::int32_t version = 0;
    std::uint64_t services = 0;
    std::int64_t timestamp = 0;
    NetAddr addr_recv;
    NetAddr addr_trans;
    std::uint64_t nonce = 0;
    std::string user_agent;
    std::int32_t start_height = 0;
    bool relay = false;
};

struct P2pMessage {
    std::string command;
    std::vector<std::uint8_t> payload;
};

struct NodePeer {
    std::string host;
    std::string ip;
    std::uint16_t port = 0;
    VersionPayload version;
};

std::string default_user_agent();
VersionPayload make_outbound_version(std::int64_t timestamp);

std::vector<std::uint8_t> serialize_version_payload(const VersionPayload& msg);
VersionPayload parse_version_payload(const std::uint8_t* data, std::size_t len);
VersionPayload parse_version_payload(const std::vector<std::uint8_t>& data);

std::vector<std::uint8_t> serialize_p2p_message(const std::string& command,
                                                const std::vector<std::uint8_t>& payload);
P2pMessage parse_p2p_message(const std::uint8_t* data, std::size_t len);
P2pMessage parse_p2p_message(const std::vector<std::uint8_t>& data);

std::vector<std::string> service_names(std::uint64_t services);
std::string format_net_ip(const std::array<std::uint8_t, 16>& ip);
std::array<std::uint8_t, 16> ipv4_mapped(std::uint8_t a, std::uint8_t b, std::uint8_t c,
                                         std::uint8_t d);

NodePeer handshake_version(const std::string& host, std::uint16_t port);
