#pragma once

#include "chain/balance_db.hpp"

#include <cstdint>
#include <functional>
#include <string>

void walk_rpc_blocks(const char* command, const std::string& host, std::uint16_t port,
                     const std::string& auth, bool incremental, std::uint32_t start_height,
                     const Hash256* expected_tip,
                     const std::function<void(const BlockEffects&)>& apply);
