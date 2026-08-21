#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/json_io.hpp"

class JsonRpc {
public:
    JsonRpc(std::string host, std::uint16_t port, std::string auth, std::string command);

    const std::string& command() const { return command_; }
    void set_timeout_ms(int ms) { timeout_ms_ = ms; }

    JsonValue call(const std::string& method, const JsonArray& params = {});

private:
    std::string host_;
    std::uint16_t port_ = 8332;
    std::string auth_;
    std::string command_;
    int timeout_ms_ = 60000;
};

struct RpcUtxo {
    std::string txid;
    std::uint32_t vout = 0;
    std::string script_hex;
    std::uint64_t amount_sats = 0;
    bool coinbase = false;
    std::uint32_t height = 0;
};

struct ScanUtxoSet {
    bool success = false;
    std::uint32_t height = 0;
    std::vector<RpcUtxo> unspents;
};

std::uint32_t rpc_getblockcount(JsonRpc& rpc);
std::string rpc_getblockhash(JsonRpc& rpc, std::uint32_t height);
std::vector<std::uint8_t> rpc_getblock(JsonRpc& rpc, const std::string& hash_hex);
ScanUtxoSet rpc_scantxoutset(JsonRpc& rpc, const std::string& address);
std::optional<std::uint64_t> rpc_estimatesmartfee_satvb(JsonRpc& rpc, int conf_target = 6);
std::string rpc_sendrawtransaction(JsonRpc& rpc, const std::string& hex);
