#pragma once

#include <cstdint>
#include <string>

#include "cli/options.hpp"

struct LoadedConfig {
    bool present = false;
    std::string rpc_host = "127.0.0.1";
    std::uint16_t rpc_port = 8332;
    std::string rpc_auth;
};

std::string resolve_config_path(const Options& opts);
LoadedConfig load_config(const Options& opts);
