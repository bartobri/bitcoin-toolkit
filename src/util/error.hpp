#pragma once

#include <stdexcept>
#include <string>

class BtkError : public std::runtime_error {
public:
    BtkError(std::string command, std::string message);
    const std::string& command() const noexcept { return command_; }

private:
    std::string command_;
};

void print_error(const BtkError& err);
