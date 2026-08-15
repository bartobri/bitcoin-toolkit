#pragma once

#include <memory>

#include "cmd/command.hpp"

std::unique_ptr<Command> make_address_command();
