/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include "opts.h"
#include "error.h"
#include "assert.h"

#define OPTS_INPUT_FORMAT    (struct opt_info){"in-format",  "lbj"}
#define OPTS_INPUT_TYPE      (struct opt_info){"in-type",    "wxrsd"}
#define OPTS_OUTPUT_FORMAT   (struct opt_info){"out-format", "LQBJ"}
#define OPTS_OUTPUT_TYPE     (struct opt_info){"out-type",   "WXDR"}
#define OPTS_STREAM          (struct opt_info){"stream",     "S"}
#define OPTS_GREP            (struct opt_info){"grep",       "G:"}
#define OPTS_COMPRESSED      (struct opt_info){"compressed", "CU"}
#define OPTS_REHASH          (struct opt_info){"rehash",     ""}
#define OPTS_HOSTNAME        (struct opt_info){"hostname",   "h:"}
#define OPTS_PORT            (struct opt_info){"port",       "p:"}
#define OPTS_CREATE          (struct opt_info){"create",     ""}
#define OPTS_CREATE_CHAINSTATE (struct opt_info){"create-from-chainstate",     ""}
#define OPTS_UPDATE          (struct opt_info){"update",     ""}
#define OPTS_CHAINSTATE_PATH (struct opt_info){"chainstate-path",    ""}
#define OPTS_BALANCE_PATH    (struct opt_info){"balance-path",   ""}
#define OPTS_BECH32          (struct opt_info){"bech32",     ""}
#define OPTS_P2PKH           (struct opt_info){"p2pkh",     ""}
#define OPTS_TESTNET         (struct opt_info){"testnet",    ""}
#define OPTS_RPC_AUTH        (struct opt_info){"rpc-auth",   ""}
#define OPTS_SET             (struct opt_info){"set",        ""}
#define OPTS_UNSET           (struct opt_info){"unset",      ""}
#define OPTS_DUMP            (struct opt_info){"dump",       ""}
#define OPTS_TRACE           (struct opt_info){"trace",      ""}
#define OPTS_MAX             30

struct opt_info {
    char *longopt;
    char *shortopt;
};

static struct option longopts[OPTS_MAX];
static char shortopts[OPTS_MAX];

int opts_add(struct opt_info, int);
int opts_process_long(opts_p, const char *, char *);

int opts_init(opts_p opts, char *command)
{
    assert(opts);
    assert(command);

    opts->input_format_list = 0;
    opts->input_format_binary = 0;
    opts->input_format_json = 0;
    opts->input_type_wif = 0;
    opts->input_type_hex = 0;
    opts->input_type_raw = 0;
    opts->input_type_string = 0;
    opts->input_type_decimal = 0;
    opts->input_type_binary = 0;
    opts->input_type_sbd = 0;
    opts->output_format_list = 0;
    opts->output_format_qrcode = 0;
    opts->output_format_binary = 0;
    opts->output_format_json = 0;
    opts->output_type_wif = 0;
    opts->output_type_hex = 0;
    opts->output_type_decimal = 0;
    opts->output_type_raw = 0;
    opts->output_type_p2pkh = 0;
    opts->output_type_p2wpkh = 0;
    opts->output_stream = 0;
    opts->output_grep = NULL;
    opts->compression_on = 0;
    opts->compression_off = 0;
    opts->network_test = 0;
    opts->rehash = NULL;
    opts->host_name = NULL;
    opts->host_service = NULL;
    opts->create = 0;
    opts->create_from_chainstate = 0;
    opts->update = 0;
    opts->chainstate_path = NULL;
    opts->balance_path = NULL;
    opts->rpc_auth = NULL;
    opts->set = NULL;
    opts->unset = NULL;
    opts->dump = 0;
    opts->trace = 0;
    opts->command = command;
    opts->subcommand = NULL;

    memset(longopts, 0, OPTS_MAX * sizeof(*longopts));
    memset(shortopts, 0, OPTS_MAX);

    if (strcmp(command, "privkey") == 0)
    {
        opts_add(OPTS_INPUT_FORMAT, required_argument);
        opts_add(OPTS_INPUT_TYPE, required_argument);
        opts_add(OPTS_OUTPUT_FORMAT, required_argument);
        opts_add(OPTS_OUTPUT_TYPE, required_argument);
        opts_add(OPTS_CREATE, no_argument);
        opts_add(OPTS_COMPRESSED, required_argument);
        opts_add(OPTS_STREAM, no_argument);
        opts_add(OPTS_REHASH, required_argument);
        opts_add(OPTS_GREP, required_argument);
        opts_add(OPTS_TESTNET, no_argument);
        opts_add(OPTS_TRACE, no_argument);
    }
    else if (strcmp(command, "pubkey") == 0)
    {
        opts_add(OPTS_INPUT_FORMAT, required_argument);
        opts_add(OPTS_INPUT_TYPE, required_argument);
        opts_add(OPTS_OUTPUT_FORMAT, required_argument);
        opts_add(OPTS_COMPRESSED, required_argument);
        opts_add(OPTS_STREAM, no_argument);
        opts_add(OPTS_GREP, required_argument);
    }
    else if (strcmp(command, "address") == 0)
    {
        opts_add(OPTS_INPUT_FORMAT, required_argument);
        opts_add(OPTS_INPUT_TYPE, required_argument);
        opts_add(OPTS_OUTPUT_FORMAT, required_argument);
        opts_add(OPTS_BECH32, no_argument);
        opts_add(OPTS_P2PKH, no_argument);
        opts_add(OPTS_STREAM, no_argument);
        opts_add(OPTS_GREP, required_argument);
        opts_add(OPTS_TRACE, no_argument);
    }
    else if (strcmp(command, "balance") == 0)
    {
        opts_add(OPTS_INPUT_FORMAT, required_argument);
        opts_add(OPTS_INPUT_TYPE, required_argument);
        opts_add(OPTS_OUTPUT_FORMAT, required_argument);
        opts_add(OPTS_HOSTNAME, required_argument);
        opts_add(OPTS_PORT, required_argument);
        opts_add(OPTS_CREATE, no_argument);
        opts_add(OPTS_CREATE_CHAINSTATE, no_argument);
        opts_add(OPTS_UPDATE, no_argument);
        opts_add(OPTS_CHAINSTATE_PATH, required_argument);
        opts_add(OPTS_BALANCE_PATH, required_argument);
        opts_add(OPTS_STREAM, no_argument);
        opts_add(OPTS_GREP, required_argument);
        opts_add(OPTS_RPC_AUTH, required_argument);
    }
    else if (strcmp(command, "node") == 0)
    {
        opts_add(OPTS_INPUT_FORMAT, required_argument);
        opts_add(OPTS_HOSTNAME, required_argument);
        opts_add(OPTS_PORT, required_argument);
        opts_add(OPTS_HOSTNAME, required_argument);
        opts_add(OPTS_STREAM, no_argument);
    }
    else if (strcmp(command, "config") == 0)
    {
        opts_add(OPTS_SET, required_argument);
        opts_add(OPTS_UNSET, required_argument);
        opts_add(OPTS_DUMP, no_argument);
    }
    else if (strcmp(command, "version") == 0)
    {
        // No options
    }
    else if (strcmp(command, "help") == 0)
    {
        // No options
    }
    else
    {
        error_log("Unknown command: %s", command);
        return -1;
    }

    return 1;
}

int opts_add(struct opt_info info, int has_arg)
{
    static int i = 0;

    longopts[i++] = (struct option){info.longopt, has_arg, NULL, 0};
    strcat(shortopts, info.shortopt);

    return 1;
}

int opts_get(opts_p opts, int argc, char *argv[])
{
    int r, o, i;
    int opt_index = 0;
    const char *optname;

    assert(opts);

    if (argc > 2 && argv[2][0] != '-')
    {
        opts->subcommand = argv[2];
    }

    // Prepend a colon to turn off getopt errors.
    for (i = strlen(shortopts); i > 0; i--)
        shortopts[i] = shortopts[i-1];
    shortopts[0] = ':';

    while ((o = getopt_long(argc, argv, shortopts, longopts, &opt_index)) != -1)
    {
        // Processing long opts
        if (o == 0)
        {
            optname = longopts[opt_index].name;

            r = opts_process_long(opts, optname, optarg);
            ERROR_CHECK_NEG(r, "Could not set option argument.");

            continue;
        }

        // Processing short opts
        switch (o)
        {
            case 'l':
                opts->input_format_list = 1;
                break;
            case 'b':
                opts->input_format_binary = 1;
                opts->input_type_binary = 1;  // TODO put this in btk init?
                break;
            case 'j':
                opts->input_format_json = 1;
                break;

            case 'w':
                opts->input_type_wif = 1;
                break;
            case 'x':
                opts->input_type_hex = 1;
                break;
            case 'r':
                opts->input_type_raw = 1;
                opts->input_format_binary = 1;
                break;
            case 's':
                opts->input_type_string = 1;
                break;
            case 'd':
                opts->input_type_decimal = 1;
                break;

            case 'L':
                opts->output_format_list = 1;
                break;
            case 'Q':
                opts->output_format_qrcode = 1;
                break;
            case 'B':
                opts->output_format_binary = 1;
                break;
            case 'J':
                opts->output_format_json = 1;
                break;

            case 'W':
                opts->output_type_wif = 1;
                break;
            case 'X':
                opts->output_type_hex = 1;
                break;
            case 'D':
                opts->output_type_decimal = 1;
                break;
            case 'R':
                opts->output_type_raw = 1;
                opts->output_format_binary = 1;
                break;

            case 'S':
                opts->output_stream = 1;
                break;

            case 'G':
                ERROR_CHECK_TRUE(opts->output_grep, "Can not use grep option more than once.");
                opts->output_grep = optarg;
                break;

            case 'C':
                opts->compression_on = 1;
                break;
            case 'U':
                opts->compression_off = 1;
                break;

            case 'h':
                ERROR_CHECK_TRUE(opts->host_name, "Can not use hostname option more than once.");
                opts->host_name = optarg;
                break;
            case 'p':
                ERROR_CHECK_TRUE(opts->host_service, "Can not use port option more than once.");
                opts->host_service = optarg;
                break;

            case ':':
                if (optopt == 0)
                {
                    error_log("Missing option argument for %s", argv[optind-1]);
                }
                else
                {
                    error_log("Missing option argument for -%c.", optopt);
                }
                return -1;

            case '?':
                if (optopt == 0)
                {
                    error_log("Invalid option: %s", argv[optind-1]);
                }
                else
                {
                    if (isprint(optopt))
                    {
                        error_log("Invalid option: -%c.", optopt);
                    }
                    else
                    {
                        error_log("Invalid option character 0x%02x.", optopt);
                    }
                }
                return -1;
        }
    }

    // Checking for extra non-options.
    // We pre-increment because the first non-option is the command.
    if (++optind < argc)
    {
        error_log("Unknown option: \"%s\".", argv[optind]);
        return -1;
    }

    // Only allow one input type
    i = 0;
    if (opts->input_type_wif) { i++; }
    if (opts->input_type_hex) { i++; }
    if (opts->input_type_raw) { i++; }
    if (opts->input_type_string) { i++; }
    if (opts->input_type_decimal) { i++; }
    if (opts->input_type_sbd) { i++; }
    if (opts->input_type_binary) { i++; }
    if (i > 1)
    {
        error_log("Can not use more than one input type option.");
        return -1;
    }

    return 1;
}

int opts_process_long(opts_p opts, const char *optname, char *optarg)
{
    if (strcmp(optname, OPTS_INPUT_FORMAT.longopt) == 0)
    {
        if (strcmp(optarg, "list") == 0)        { opts->input_format_list = 1; }
        else if (strcmp(optarg, "binary") == 0) { opts->input_format_binary = 1; opts->input_type_binary = 1; }
        else if (strcmp(optarg, "json") == 0)   { opts->input_format_json = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_INPUT_TYPE.longopt) == 0)
    {
        if (strcmp(optarg, "wif") == 0)            { opts->input_type_wif = 1; }
        else if (strcmp(optarg, "hex") == 0)       { opts->input_type_hex = 1; }
        else if (strcmp(optarg, "raw") == 0)       { opts->input_type_raw = 1; opts->input_format_binary = 1; }
        else if (strcmp(optarg, "string") == 0)    { opts->input_type_string = 1; }
        else if (strcmp(optarg, "decimal") == 0)   { opts->input_type_decimal = 1; }
        else if (strcmp(optarg, "binary") == 0)    { opts->input_type_binary = 1; opts->input_format_binary = 1; }
        else if (strcmp(optarg, "sbd") == 0)       { opts->input_type_sbd = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_OUTPUT_FORMAT.longopt) == 0)
    {
        if (strcmp(optarg, "list") == 0)            { opts->output_format_list = 1; }
        else if (strcmp(optarg, "binary") == 0)     { opts->output_format_binary = 1; }
        else if (strcmp(optarg, "qrcode") == 0)     { opts->output_format_qrcode = 1; }
        else if (strcmp(optarg, "json") == 0)       { opts->output_format_json = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_OUTPUT_TYPE.longopt) == 0)
    {
        if (strcmp(optarg, "wif") == 0)             { opts->output_type_wif = 1; }
        else if (strcmp(optarg, "hex") == 0)        { opts->output_type_hex = 1; }
        else if (strcmp(optarg, "decimal") == 0)    { opts->output_type_decimal = 1; }
        else if (strcmp(optarg, "raw") == 0)        { opts->output_type_raw = 1; opts->output_format_binary = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_STREAM.longopt) == 0)
    {
        opts->output_stream = 1;
    }

    else if (strcmp(optname, OPTS_GREP.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->output_grep, "Can not use grep option more than once.");
        opts->output_grep = optarg;
    }

    else if (strcmp(optname, OPTS_COMPRESSED.longopt) == 0)
    {
        if (strcmp(optarg, "true") == 0)            { opts->compression_on = 1; }
        else if (strcmp(optarg, "false") == 0)      { opts->compression_off = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_REHASH.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->rehash, "Can not use rehash option more than once.");
        opts->rehash = optarg;
    }

    else if (strcmp(optname, OPTS_HOSTNAME.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->host_name, "Can not use hostname option more than once.");
        opts->host_name = optarg;
    }

    else if (strcmp(optname, OPTS_PORT.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->host_service, "Can not use port option more than once.");
        opts->host_service = optarg;
    }

    else if (strcmp(optname, OPTS_CREATE.longopt) == 0)
    {
        opts->create = 1;
    }

    else if (strcmp(optname, OPTS_CREATE_CHAINSTATE.longopt) == 0)
    {
        opts->create_from_chainstate = 1;
    }

    else if (strcmp(optname, OPTS_UPDATE.longopt) == 0)
    {
        opts->update = 1;
    }

    else if (strcmp(optname, OPTS_CHAINSTATE_PATH.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->chainstate_path, "Can not use input path option more than once.");
        opts->chainstate_path = optarg;
    }

    else if (strcmp(optname, OPTS_BALANCE_PATH.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->balance_path, "Can not use output path option more than once.");
        opts->balance_path = optarg;
    }

    else if (strcmp(optname, OPTS_P2PKH.longopt) == 0)
    {
        opts->output_type_p2pkh = 1;
    }

    else if (strcmp(optname, OPTS_BECH32.longopt) == 0)
    {
        opts->output_type_p2wpkh = 1;
    }

    else if (strcmp(optname, OPTS_TESTNET.longopt) == 0)
    {
        opts->network_test = 1;
    }

    else if (strcmp(optname, OPTS_RPC_AUTH.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->rpc_auth, "Can not use report option more than once.");
        opts->rpc_auth = optarg;
    }

    else if (strcmp(optname, OPTS_SET.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->set, "Can not use set option more than once.");
        opts->set = optarg;
    }

    else if (strcmp(optname, OPTS_UNSET.longopt) == 0)
    {
        ERROR_CHECK_TRUE(opts->unset, "Can not use unset option more than once.");
        opts->unset = optarg;
    }

    else if (strcmp(optname, OPTS_DUMP.longopt) == 0)
    {
        opts->dump = 1;
    }

    else if (strcmp(optname, OPTS_TRACE.longopt) == 0)
    {
        opts->trace = 1;
    }

    return 1;
}