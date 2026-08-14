#include "util/error.hpp"

#include <iostream>

BtkError::BtkError(std::string command, std::string message)
    : std::runtime_error(std::move(message)), command_(std::move(command)) {}

void print_error(const BtkError& err) {
    if (err.command().empty()) {
        std::cerr << "btk: " << err.what() << '\n';
    } else {
        std::cerr << "btk " << err.command() << ": " << err.what() << '\n';
    }
}
