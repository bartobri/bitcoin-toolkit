/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "opts.h"
#include "error.h"
#include "assert.h"

#define OUTPUT_SET_FORMAT(x)       if (opts->output_format == OPTS_OUTPUT_FORMAT_NONE) { opts->output_format = x; } else { error_log("Cannot use multiple output format options."); return -1; }
#define OUTPUT_SET_TYPE(x)         if (opts->output_type == OPTS_OUTPUT_TYPE_NONE) { opts->output_type = x; } else { error_log("Cannot use multiple output type options."); return -1; }
#define OUTPUT_SET_COMPRESSION(x)  if (opts->compression == OPTS_OUTPUT_COMPRESSION_NONE) { opts->compression = x; } else { opts->compression = OPTS_OUTPUT_COMPRESSION_BOTH; }
#define OUTPUT_SET_NETWORK(x)      if (opts->network == OPTS_OUTPUT_NETWORK_NONE) { opts->network = x; } else { error_log("Cannot use multiple network options."); return -1; }
#define OUTPUT_SET_REHASHES(x)     if (opts->rehashes == OPTS_OUTPUT_REHASHES_NONE) { opts->rehashes = x; } else { error_log("Cannot use multiple rehash options."); return -1; }
#define HOST_SET_NAME(x)           if (opts->host_name == OPTS_HOST_NAME_NONE) { opts->host_name = x; } else { error_log("Cannot use multiple host name options."); return -1; }
#define HOST_SET_PORT(x)           if (opts->host_port == OPTS_HOST_PORT_NONE) { opts->host_port = x; } else { error_log("Cannot use multiple host port options."); return -1; }
#define CREATE_SET_CREATE(x)       if (opts->create == OPTS_CREATE_FALSE) { opts->create = x; } else { error_log("Cannot use multiple create options."); return -1; }
#define INPUT_SET_PATH(x)          if (opts->input_path == OPTS_INPUT_PATH_NONE) { opts->input_path = x; } else { error_log("Cannot use multiple input path options."); return -1; }
#define OUTPUT_SET_PATH(x)         if (opts->output_path == OPTS_OUTPUT_PATH_NONE) { opts->output_path = x; } else { error_log("Cannot use multiple output path options."); return -1; }

int opts_init(opts_p opts)
{
    assert(opts);

    opts->input_format_list = 0;
    opts->input_format_binary = 0;
    opts->input_type_wif = 0;
    opts->input_type_hex = 0;
    opts->input_type_raw = 0;
    opts->input_type_string = 0;
    opts->input_type_decimal = 0;
    opts->input_type_binary = 0;
    opts->input_type_sbd = 0;
    opts->input_type_vanity = 0;
    opts->output_format_list = 0;
    opts->output_format_binary = 0;
    opts->output_type = OPTS_OUTPUT_TYPE_NONE;
    opts->compression = OPTS_OUTPUT_COMPRESSION_NONE;
    opts->network = OPTS_OUTPUT_NETWORK_NONE;
    opts->rehashes = OPTS_OUTPUT_REHASHES_NONE;
    opts->host_name = OPTS_HOST_NAME_NONE;
    opts->host_port = OPTS_HOST_PORT_NONE;
    opts->create = OPTS_CREATE_FALSE;
    opts->input_path = OPTS_INPUT_PATH_NONE;
    opts->output_path = OPTS_OUTPUT_PATH_NONE;
    opts->subcommand = OPTS_SUBCOMMAND_NONE;

    return 1;
}

int opts_get(opts_p opts, int argc, char *argv[], char *opts_string)
{
    int r, o;

    assert(opts);
    assert(opts_string);

    r = opts_init(opts);
    ERROR_CHECK_NEG(r, "Could not initialize opts.");

    if (argc > 2 && argv[2][0] != '-')
    {
        opts->subcommand = argv[2];
    }

    // Turn off getopt errors. I print my own errors.
    opterr = 0;

    while ((o = getopt(argc, argv, opts_string)) != -1)
    {
        switch (o)
        {
            case 'l':
                opts->input_format_list = 1;
                break;
            case 'b':
                opts->input_format_binary = 1;
                opts->input_type_binary = 1;
                break;

            case 'w':
                opts->input_type_wif = 1;
                break;
            case 'h':
                opts->input_type_hex = 1;
                break;
            case 'r':
                opts->input_type_raw = 1;
                break;
            case 's':
                opts->input_type_string = 1;
                break;
            case 'd':
                opts->input_type_decimal = 1;
                break;
            case 'x':
                opts->input_type_sbd = 1;
                break;
            case 'v':
                opts->input_type_vanity = 1;
                break;

            case 'L':
                opts->output_format_list = 1;
                break;

            case 'W':
                OUTPUT_SET_TYPE(OPTS_OUTPUT_TYPE_WIF);
                break;
            case 'H':
                OUTPUT_SET_TYPE(OPTS_OUTPUT_TYPE_HEX);
                break;
            case 'D':
                OUTPUT_SET_TYPE(OPTS_OUTPUT_TYPE_DECIMAL);
                break;
            case 'P':
                OUTPUT_SET_TYPE(OPTS_OUTPUT_TYPE_P2PKH);
                break;
            case 'B':
                OUTPUT_SET_TYPE(OPTS_OUTPUT_TYPE_P2WPKH);
                break;

            case 'C':
                OUTPUT_SET_COMPRESSION(OPTS_OUTPUT_COMPRESSION_TRUE);
                break;
            case 'U':
                OUTPUT_SET_COMPRESSION(OPTS_OUTPUT_COMPRESSION_FALSE);
                break;

            case 'M':
                OUTPUT_SET_NETWORK(OPTS_OUTPUT_NETWORK_MAINNET);
                break;
            case 'T':
                OUTPUT_SET_NETWORK(OPTS_OUTPUT_NETWORK_TESTNET);
                break;

            case 'R':
                OUTPUT_SET_REHASHES(optarg);
                break;

            case 'n':
                HOST_SET_NAME(optarg);
                break;
            case 'p':
                HOST_SET_PORT(atoi(optarg));
                break;

            case 'c':
                CREATE_SET_CREATE(OPTS_CREATE_TRUE);
                break;

            case 'f':
                INPUT_SET_PATH(optarg);
                break;
            case 'F':
                OUTPUT_SET_PATH(optarg);
                break;

            case '?':
                if (isprint(optopt))
                {
                    error_log("Invalid option or missing option argument: '-%c'.", optopt);
                }
                else
                {
                    error_log("Invalid option character '\\x%x'.", optopt);
                }
                return -1;
        }
    }

    return 1;
}