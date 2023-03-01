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

#define OPTS_INPUT_FORMAT    "in-format"
#define OPTS_INPUT_TYPE      "in-type"
#define OPTS_OUTPUT_FORMAT   "out-format"
#define OPTS_OUTPUT_TYPE     "out-type"
#define OPTS_STREAM          "stream"
#define OPTS_GREP            "grep"
#define OPTS_COMPRESSED      "compressed"
#define OPTS_REHASH          "rehash"
#define OPTS_NETWORK         "network"
#define OPTS_HOSTNAME        "hostname"
#define OPTS_PORT            "port"
#define OPTS_CREATE          "create"
#define OPTS_INPUT_FILE      "in-file"
#define OPTS_OUTPUT_FILE     "out-file"
#define OPTS_BECH32          "bech32"
#define OPTS_ADD(x, y)       longopts[i++] = (struct option){x,  y, NULL, 0};
#define OPTS_MAX             20

static struct option longopts[OPTS_MAX];

int opts_long_set_arg(opts_p, const char *, char *);

int opts_init(opts_p opts, char *command)
{
    int i = 0;

    assert(opts);
    assert(command);

    opts->input_format_list = 0;
    opts->input_format_binary = 0;
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
    opts->rehashes = NULL;
    opts->host_name = NULL;
    opts->host_service = NULL;
    opts->create = 0;
    opts->input_path = NULL;
    opts->output_path = NULL;
    opts->command = command;
    opts->subcommand = NULL;

    memset(longopts, 0, OPTS_MAX * sizeof(*longopts));

    if (strcmp(command, "privkey") == 0)
    {
        OPTS_ADD(OPTS_INPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_INPUT_TYPE, required_argument);
        OPTS_ADD(OPTS_OUTPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_OUTPUT_TYPE, required_argument);
        OPTS_ADD(OPTS_CREATE, no_argument);
        OPTS_ADD(OPTS_COMPRESSED, required_argument);
        OPTS_ADD(OPTS_NETWORK, required_argument);
        OPTS_ADD(OPTS_STREAM, no_argument);
        OPTS_ADD(OPTS_REHASH, required_argument);
        OPTS_ADD(OPTS_GREP, required_argument);
    }
    else if (strcmp(command, "pubkey") == 0)
    {
        OPTS_ADD(OPTS_INPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_INPUT_TYPE, required_argument);
        OPTS_ADD(OPTS_OUTPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_COMPRESSED, required_argument);
        OPTS_ADD(OPTS_STREAM, no_argument);
        OPTS_ADD(OPTS_GREP, required_argument);
    }
    else if (strcmp(command, "address") == 0)
    {
        OPTS_ADD(OPTS_INPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_INPUT_TYPE, required_argument);
        OPTS_ADD(OPTS_OUTPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_BECH32, no_argument);
        OPTS_ADD(OPTS_STREAM, no_argument);
        OPTS_ADD(OPTS_GREP, required_argument);
    }
    else if (strcmp(command, "balance") == 0)
    {
        OPTS_ADD(OPTS_INPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_INPUT_TYPE, required_argument);
        OPTS_ADD(OPTS_OUTPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_HOSTNAME, required_argument);
        OPTS_ADD(OPTS_PORT, required_argument);
        OPTS_ADD(OPTS_CREATE, no_argument);
        OPTS_ADD(OPTS_INPUT_FILE, required_argument);
        OPTS_ADD(OPTS_OUTPUT_FILE, required_argument);
        OPTS_ADD(OPTS_STREAM, no_argument);
        OPTS_ADD(OPTS_GREP, required_argument);
    }
    else if (strcmp(command, "node") == 0)
    {
        OPTS_ADD(OPTS_INPUT_FORMAT, required_argument);
        OPTS_ADD(OPTS_HOSTNAME, required_argument);
        OPTS_ADD(OPTS_PORT, required_argument);
        OPTS_ADD(OPTS_HOSTNAME, required_argument);
        OPTS_ADD(OPTS_STREAM, no_argument);
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

int opts_get(opts_p opts, int argc, char *argv[], char *opts_string)
{
    int r, o;
    int opt_index = 0;
    char opt_string_noerror[BUFSIZ];
    const char *optname;

    assert(opts);
    assert(opts_string);

    if (argc > 2 && argv[2][0] != '-')
    {
        opts->subcommand = argv[2];
    }

    // Turn off getopt errors. 
    memset(opt_string_noerror, 0, BUFSIZ);
    strcpy(opt_string_noerror, ":");
    strcat(opt_string_noerror, opts_string);

    while ((o = getopt_long(argc, argv, opt_string_noerror, longopts, &opt_index)) != -1)
    {
        // Processing long opts
        if (o == 0)
        {
            optname = longopts[opt_index].name;

            r = opts_long_set_arg(opts, optname, optarg);
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

            case 'w':
                opts->input_type_wif = 1;
                break;
            case 'h':
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

            case 'W':
                opts->output_type_wif = 1;
                break;
            case 'H':
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
                opts->output_grep = optarg;
                break;

            case 'C':
                opts->compression_on = 1;
                break;
            case 'U':
                opts->compression_off = 1;
                break;

            case 'T':
                opts->network_test = 1;
                break;

            case 'n':
                opts->host_name = optarg;
                break;
            case 'p':
                opts->host_service = optarg;
                break;

            case 'c':
                opts->create = 1;
                break;

            case 'f':
                opts->input_path = optarg;
                break;
            case 'F':
                opts->output_path = optarg;
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

    return 1;
}

int opts_long_set_arg(opts_p opts, const char *optname, char *optarg)
{
    if (strcmp(optname, OPTS_INPUT_FORMAT) == 0)
    {
        if (strcmp(optarg, "list") == 0)        { opts->input_format_list = 1; }
        else if (strcmp(optarg, "binary") == 0) { opts->input_format_binary = 1; opts->input_type_binary = 1; }
        else if (strcmp(optarg, "json") == 0)   { }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_INPUT_TYPE) == 0)
    {
        if (strcmp(optarg, "wif") == 0)            { opts->input_type_wif = 1; }
        else if (strcmp(optarg, "hex") == 0)       { opts->input_type_hex = 1; }
        else if (strcmp(optarg, "raw") == 0)       { opts->input_type_raw = 1; opts->input_format_binary = 1; }
        else if (strcmp(optarg, "string") == 0)    { opts->input_type_string = 1; }
        else if (strcmp(optarg, "decimal") == 0)   { opts->input_type_decimal = 1; }
        else if (strcmp(optarg, "sbd") == 0)       { opts->input_type_sbd = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_OUTPUT_FORMAT) == 0)
    {
        if (strcmp(optarg, "list") == 0)            { opts->output_format_list = 1; }
        else if (strcmp(optarg, "binary") == 0)     { opts->output_format_binary = 1; }
        else if (strcmp(optarg, "qrcode") == 0)     { opts->output_format_qrcode = 1; }
        else if (strcmp(optarg, "json") == 0)       { }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_OUTPUT_TYPE) == 0)
    {
        if (strcmp(optarg, "wif") == 0)             { opts->output_type_wif = 1; }
        else if (strcmp(optarg, "hex") == 0)        { opts->output_type_hex = 1; }
        else if (strcmp(optarg, "decimal") == 0)    { opts->output_type_decimal = 1; }
        else if (strcmp(optarg, "raw") == 0)        { opts->output_type_raw = 1; opts->output_format_binary = 1; }
        else if (strcmp(optarg, "p2pkh") == 0)      { opts->output_type_p2pkh = 1; }
        else if (strcmp(optarg, "p2wpkh") == 0)     { opts->output_type_p2wpkh = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_STREAM) == 0)
    {
        opts->output_stream = 1;
    }

    else if (strcmp(optname, OPTS_GREP) == 0)
    {
        opts->output_grep = optarg;
    }

    else if (strcmp(optname, OPTS_COMPRESSED) == 0)
    {
        if (strcmp(optarg, "true") == 0)            { opts->compression_on = 1; }
        else if (strcmp(optarg, "false") == 0)      { opts->compression_off = 1; }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_REHASH) == 0)
    {
        opts->rehashes = optarg;    // TODO - change to 'rehash'
    }

    else if (strcmp(optname, OPTS_NETWORK) == 0)
    {
        if (strcmp(optarg, "test") == 0)            { opts->network_test = 1; }
        else if (strcmp(optarg, "main") == 0)       { }
        else
        {
            error_log("Invalid argument for option --%s.", optname);
            return -1;
        }
    }

    else if (strcmp(optname, OPTS_HOSTNAME) == 0)
    {
        opts->host_name = optarg;
    }

    else if (strcmp(optname, OPTS_PORT) == 0)
    {
        opts->host_service = optarg;
    }

    else if (strcmp(optname, OPTS_CREATE) == 0)
    {
        opts->create = 1;
    }

    else if (strcmp(optname, OPTS_INPUT_FILE) == 0)
    {
        opts->input_path = optarg;
    }

    else if (strcmp(optname, OPTS_OUTPUT_FILE) == 0)
    {
        opts->output_path = optarg;
    }

    else if (strcmp(optname, OPTS_BECH32) == 0)
    {
        opts->output_type_p2wpkh = 1;
    }

    return 1;
}