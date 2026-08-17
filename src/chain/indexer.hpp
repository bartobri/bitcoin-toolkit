#pragma once

#include "chain/balance_db.hpp"
#include "chain/transaction.hpp"

BlockEffects effects_from_block(const Block& block, std::uint32_t height);
