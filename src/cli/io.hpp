#pragma once

#include <functional>
#include <iosfwd>
#include <string>

#include "cli/options.hpp"
#include "core/json_io.hpp"

using ItemHandler = std::function<void(const JsonObject&)>;

// Wrap a positional or plain-line string as a typed or bare object.
JsonObject item_from_text(const std::string& text);

// Incremental stdin walk. Invokes handler once per item.
void for_each_stdin_item(const Options& opts, const ItemHandler& handler);
