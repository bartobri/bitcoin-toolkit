#include "cli/options.hpp"

#include "util/error.hpp"

#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

void OptionSpec::add(char short_name, const char* long_name, bool has_arg) {
    add(short_name, long_name, has_arg ? FlagArg::Required : FlagArg::None);
}

void OptionSpec::add(char short_name, const char* long_name, FlagArg arg) {
    flags_.push_back(FlagSpec{short_name, long_name, arg});
}

void add_global_flags(OptionSpec& spec) {
    spec.add('h', "help", false);
    spec.add('V', "version", false);
    spec.add(0, "config", true);
    spec.add('n', "network", true);
    spec.add('o', "out", true);
    spec.add(0, "in", true);
    spec.add('s', "stream", false);
    spec.add('c', "count", true);
}

namespace {

bool is_global_arg_flag(const std::string& arg) {
    static const char* kNames[] = {"--config", "--network", "-n", "--out", "-o", "--in", "--count",
                                   "-c", nullptr};
    // Strip =value
    const std::string name = arg.substr(0, arg.find('='));
    for (int i = 0; kNames[i]; ++i) {
        if (name == kNames[i]) {
            return true;
        }
    }
    return false;
}

}  // namespace

std::string find_command_name(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                return argv[i + 1];
            }
            break;
        }
        if (!a.empty() && a[0] != '-') {
            return a;
        }
        if (is_global_arg_flag(a) && a.find('=') == std::string::npos) {
            ++i;
        }
    }
    return {};
}

void parse_argv(int argc, char** argv, const OptionSpec& spec, Options& opts) {
    std::vector<option> longopts;
    std::string shorts;
    shorts.push_back(':');  // silent errors; we report them

    for (const FlagSpec& f : spec.flags()) {
        option o{};
        o.name = f.long_name;
        if (f.arg == FlagArg::Required) {
            o.has_arg = required_argument;
        } else if (f.arg == FlagArg::Optional) {
            o.has_arg = optional_argument;
        } else {
            o.has_arg = no_argument;
        }
        o.flag = nullptr;
        o.val = f.short_name ? f.short_name : 0;
        longopts.push_back(o);
        if (f.short_name) {
            shorts.push_back(f.short_name);
            if (f.arg == FlagArg::Required) {
                shorts.push_back(':');
            } else if (f.arg == FlagArg::Optional) {
                shorts.push_back(':');
                shorts.push_back(':');
            }
        }
    }
    longopts.push_back(option{nullptr, 0, nullptr, 0});

    // Rebuild argv without the command token so getopt_long sees only flags/positionals.
    std::vector<char*> fake;
    fake.push_back(argv[0]);
    bool seen_cmd = false;
    for (int i = 1; i < argc; ++i) {
        if (!seen_cmd && argv[i][0] != '-' && std::strcmp(argv[i], "--") != 0) {
            if (opts.command == argv[i]) {
                seen_cmd = true;
                continue;
            }
        }
        fake.push_back(argv[i]);
    }
    int fac = static_cast<int>(fake.size());

    optind = 1;
    opterr = 0;
    int opt;
    int idx = 0;
    while ((opt = getopt_long(fac, fake.data(), shorts.c_str(), longopts.data(), &idx)) != -1) {
        if (opt == '?') {
            const char* bad = (optopt ? nullptr : fake[static_cast<std::size_t>(optind - 1)]);
            if (optopt) {
                throw BtkError(opts.command, std::string("unknown option '-") +
                                                 static_cast<char>(optopt) + "'");
            }
            throw BtkError(opts.command, std::string("unknown option '") +
                                             (bad ? bad : "?") + "'");
        }
        if (opt == ':') {
            throw BtkError(opts.command, "missing option argument");
        }

        const char* long_name = nullptr;
        if (opt == 0) {
            long_name = longopts[static_cast<std::size_t>(idx)].name;
        } else {
            for (const FlagSpec& f : spec.flags()) {
                if (f.short_name == opt) {
                    long_name = f.long_name;
                    break;
                }
            }
        }
        if (long_name == nullptr) {
            throw BtkError(opts.command, "unknown option");
        }

        const std::string name = long_name;
        if (name == "help") {
            opts.help = true;
        } else if (name == "version") {
            opts.version = true;
        } else if (name == "config") {
            opts.config_path = optarg ? optarg : "";
        } else if (name == "network") {
            if (!optarg || !is_network_name(optarg)) {
                throw BtkError(opts.command, "invalid --network");
            }
            opts.network = parse_network(optarg);
            opts.network_set = true;
        } else if (name == "out") {
            if (!optarg) {
                throw BtkError(opts.command, "invalid --out");
            }
            const std::string v = optarg;
            if (v == "ndjson") {
                opts.out = OutFormat::Ndjson;
            } else if (v == "json") {
                opts.out = OutFormat::Json;
            } else if (v == "plain") {
                opts.out = OutFormat::Plain;
            } else {
                throw BtkError(opts.command, "invalid --out");
            }
        } else if (name == "in") {
            if (!optarg) {
                throw BtkError(opts.command, "invalid --in");
            }
            const std::string v = optarg;
            if (v == "auto") {
                opts.in = InFormat::Auto;
            } else if (v == "ndjson") {
                opts.in = InFormat::Ndjson;
            } else if (v == "json") {
                opts.in = InFormat::Json;
            } else if (v == "plain") {
                opts.in = InFormat::Plain;
            } else {
                throw BtkError(opts.command, "invalid --in");
            }
        } else if (name == "stream") {
            opts.stream = true;
        } else if (name == "count") {
            if (!optarg || !*optarg) {
                throw BtkError(opts.command, "invalid --count");
            }
            char* end = nullptr;
            const long v = std::strtol(optarg, &end, 10);
            if (end == optarg || *end != '\0' || v < 1) {
                throw BtkError(opts.command, "invalid --count");
            }
            opts.count = static_cast<std::uint64_t>(v);
            opts.count_set = true;
        } else if (name == "new") {
            opts.flag_new = true;
        } else if (name == "compressed") {
            opts.flag_compressed = true;
        } else if (name == "uncompressed") {
            opts.flag_uncompressed = true;
        } else if (name == "from") {
            opts.from = optarg ? optarg : "";
        } else if (name == "encoding") {
            opts.encoding = optarg ? optarg : "";
        } else if (name == "type") {
            opts.types.push_back(optarg ? optarg : "");
        } else if (name == "match") {
            if (opts.match_set) {
                throw BtkError(opts.command, "cannot pass --match more than once");
            }
            opts.match_set = true;
            opts.match = optarg ? optarg : "";
        } else if (name == "ignore-case") {
            opts.ignore_case = true;
        } else if (name == "source") {
            opts.source = true;
        } else if (name == "host") {
            opts.host = optarg ? optarg : "";
        } else if (name == "verbose") {
            opts.verbose = true;
        } else if (name == "force") {
            opts.force = true;
        } else if (name == "build") {
            opts.build = true;
        } else if (name == "update") {
            opts.update = true;
        } else if (name == "from-rpc") {
            opts.from_rpc = true;
        } else if (name == "from-chainstate") {
            opts.from_chainstate = true;
        } else if (name == "path") {
            opts.path = optarg ? optarg : "";
        } else {
            throw BtkError(opts.command, std::string("unknown option '--") + name + "'");
        }
    }

    for (int i = optind; i < fac; ++i) {
        if (std::strcmp(fake[static_cast<std::size_t>(i)], "--") == 0) {
            continue;
        }
        opts.positionals.push_back(fake[static_cast<std::size_t>(i)]);
    }
}
