#pragma once

#include <memory>

#include "cmd/command.hpp"

std::unique_ptr<Command> make_inflow_command();
