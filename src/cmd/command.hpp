#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "cli/options.hpp"
#include "core/json_io.hpp"

class Command {
public:
    virtual ~Command() = default;
    virtual const char* name() const = 0;
    virtual const char* summary() const = 0;
    virtual const char* help() const { return ""; }
    virtual void register_options(OptionSpec&) const = 0;
    virtual bool is_generator(const Options&) const = 0;
    virtual void init(Options&) {}
    virtual std::vector<JsonObject> run(const Options&,
                                        const std::optional<JsonObject>&) = 0;
};

void register_command(std::unique_ptr<Command> cmd);
void register_builtin_commands();
Command* find_command(const std::string& name);
std::vector<Command*> all_commands();
