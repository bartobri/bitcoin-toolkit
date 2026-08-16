#include "cli/dispatcher.hpp"

#include "cli/io.hpp"
#include "cli/options.hpp"
#include "cli/output.hpp"
#include "cmd/address.hpp"
#include "cmd/command.hpp"
#include "cmd/node.hpp"
#include "cmd/privkey.hpp"
#include "cmd/pubkey.hpp"
#include "util/error.hpp"
#include "version.hpp"

#include <csignal>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace {

std::vector<std::unique_ptr<Command>>& registry() {
    static std::vector<std::unique_ptr<Command>> cmds;
    return cmds;
}

volatile std::sig_atomic_t g_stop = 0;

void on_sigint(int) { g_stop = 1; }

void run_transformer(Command& cmd, const Options& opts, OutputWriter& out) {
    auto handle = [&](const JsonObject& item) {
        const std::vector<JsonObject> produced = cmd.run(opts, item);
        for (const JsonObject& o : produced) {
            out.write(o);
        }
    };

    if (!opts.positionals.empty()) {
        for (const std::string& p : opts.positionals) {
            handle(item_from_text(p));
        }
        return;
    }
    for_each_stdin_item(opts, handle);
}

void run_generator(Command& cmd, const Options& opts, OutputWriter& out) {
    const bool repeatable = opts.flag_new;
    const bool infinite = repeatable && opts.stream && !opts.count_set;
    const std::uint64_t n = opts.count_set ? opts.count : 1;
    std::uint64_t i = 0;
    while (!g_stop && (infinite || i < n)) {
        const std::vector<JsonObject> produced = cmd.run(opts, std::nullopt);
        for (const JsonObject& o : produced) {
            out.write(o);
        }
        ++i;
        if (!repeatable) {
            break;
        }
    }
}

}  // namespace

void register_command(std::unique_ptr<Command> cmd) {
    registry().push_back(std::move(cmd));
}

void register_builtin_commands() {
    static bool done = false;
    if (done) {
        return;
    }
    done = true;
    register_command(make_privkey_command());
    register_command(make_pubkey_command());
    register_command(make_address_command());
    register_command(make_node_command());
}

Command* find_command(const std::string& name) {
    for (auto& c : registry()) {
        if (name == c->name()) {
            return c.get();
        }
    }
    return nullptr;
}

std::vector<Command*> all_commands() {
    std::vector<Command*> out;
    out.reserve(registry().size());
    for (auto& c : registry()) {
        out.push_back(c.get());
    }
    return out;
}

void print_overview(std::ostream& out) {
    out << "btk — Bitcoin Toolkit " << BTK_VERSION_STRING << "\n\n";
    out << "Usage:\n";
    out << "  btk [--config PATH] <command> [options]\n\n";
    out << "Commands:\n";
    for (Command* c : all_commands()) {
        out << "  " << std::left << std::setw(10) << c->name() << c->summary() << '\n';
    }
    out << "\nOutput is one JSON object per line (ndjson) unless --out json|plain.\n";
    out << "Pipes compose:  btk privkey --new | btk address --type p2wpkh\n\n";
    out << "See 'btk <command> --help'.\n";
}

int dispatch(int argc, char** argv) {
    register_builtin_commands();

    Options opts;
    opts.command = find_command_name(argc, argv);

    OptionSpec spec;
    add_global_flags(spec);
    Command* cmd = nullptr;
    if (!opts.command.empty()) {
        cmd = find_command(opts.command);
        if (cmd != nullptr) {
            cmd->register_options(spec);
        }
    }

    if (!opts.command.empty() && cmd == nullptr) {
        std::cerr << "btk: unknown command '" << opts.command << "'\n";
        std::cerr << "See 'btk --help' for a list of commands.\n";
        return 1;
    }

    parse_argv(argc, argv, spec, opts);

    if (opts.help) {
        if (cmd != nullptr) {
            const char* text = cmd->help();
            if (text != nullptr && text[0] != '\0') {
                std::cout << text;
                return 0;
            }
        }
        print_overview(std::cout);
        return 0;
    }

    if (opts.version) {
        OutputWriter out(opts);
        out.write(version_object());
        out.finish();
        return 0;
    }

    if (opts.command.empty()) {
        print_overview(std::cerr);
        return 1;
    }

    cmd->init(opts);

    std::signal(SIGINT, on_sigint);

    OutputWriter out(opts);
    if (cmd->is_generator(opts)) {
        run_generator(*cmd, opts, out);
    } else {
        run_transformer(*cmd, opts, out);
    }
    out.finish();
    return 0;
}
