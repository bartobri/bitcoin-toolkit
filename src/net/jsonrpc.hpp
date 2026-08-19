#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/json_io.hpp"

class JsonRpc {
public:
    JsonRpc(std::string host, std::uint16_t port, std::string auth, std::string command);

    const std::string& command() const { return command_; }

    JsonValue call(const std::string& method, const JsonArray& params = {});

private:
    std::string host_;
    std::uint16_t port_ = 8332;
    std::string auth_;
    std::string command_;
};

std::uint32_t rpc_getblockcount(JsonRpc& rpc);
std::string rpc_getblockhash(JsonRpc& rpc, std::uint32_t height);
std::vector<std::uint8_t> rpc_getblock(JsonRpc& rpc, const std::string& hash_hex);
