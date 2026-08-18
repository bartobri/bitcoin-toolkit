#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/network.hpp"

enum class OutFormat { Ndjson, Json, Plain };
enum class InFormat { Auto, Ndjson, Json, Plain };
enum class FlagArg { None, Required, Optional };

struct FlagSpec {
    char short_name;  // 0 if none
    const char* long_name;
    FlagArg arg;
};

class OptionSpec {
public:
    void add(char short_name, const char* long_name, bool has_arg);
    void add(char short_name, const char* long_name, FlagArg arg);
    const std::vector<FlagSpec>& flags() const { return flags_; }

private:
    std::vector<FlagSpec> flags_;
};

struct Options {
    std::string command;
    bool help = false;
    bool version = false;
    std::string config_path;
    Network network = Network::Main;
    bool network_set = false;
    OutFormat out = OutFormat::Ndjson;
    InFormat in = InFormat::Auto;
    bool stream = false;
    bool count_set = false;
    std::uint64_t count = 1;
    std::vector<std::string> positionals;

    // Command-specific flags filled by later phases via extras.
    bool flag_new = false;
    bool flag_compressed = false;
    bool flag_uncompressed = false;
    std::string from;  // wif|hex|dec|text|file; empty = guess
    std::string encoding;
    std::vector<std::string> types;
    std::string match;
    bool match_set = false;
    bool ignore_case = false;
    bool source = false;
    bool skip_incompatible = false;
    std::string host;
    bool port_set = false;
    std::uint16_t port = 0;
    bool verbose = false;
    bool force = false;
    bool sync = false;
    std::string rpc_auth;
};

void add_global_flags(OptionSpec& spec);
void parse_argv(int argc, char** argv, const OptionSpec& spec, Options& opts);
std::string find_command_name(int argc, char** argv);
