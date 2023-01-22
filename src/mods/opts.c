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
    opts->output_type_hex = 0;
    opts->output_type_decimal = 0;
    opts->output_type_p2pkh = 0;
    opts->output_type_p2wpkh = 0;
    opts->compression_on = 0;
    opts->compression_off = 0;
    opts->network_test = 0;
    opts->rehashes = NULL;
    opts->host_name = NULL;
    opts->host_port = 0;

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

            case 'H':
                opts->output_type_hex = 1;
                break;
            case 'D':
                opts->output_type_decimal = 1;
                break;
            case 'P':
                opts->output_type_p2pkh = 1;
                break;
            case 'B':
                opts->output_type_p2wpkh = 1;
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

            case 'R':
                opts->rehashes = optarg;
                break;

            case 'n':
                opts->host_name = optarg;
                break;
            case 'p':
                opts->host_port = atoi(optarg);
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