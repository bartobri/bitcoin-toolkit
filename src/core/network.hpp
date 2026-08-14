#pragma once

#include <string>

enum class Network { Main, Test };

inline const char* network_name(Network n) {
    return n == Network::Test ? "testnet" : "mainnet";
}

inline Network parse_network(const std::string& s) {
    if (s == "mainnet") {
        return Network::Main;
    }
    if (s == "testnet") {
        return Network::Test;
    }
    return Network::Main;  // caller should validate first
}

inline bool is_network_name(const std::string& s) {
    return s == "mainnet" || s == "testnet";
}
