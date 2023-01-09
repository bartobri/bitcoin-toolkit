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

#define INPUT_SET_FORMAT(x)        if (opts->input_format == OPTS_INPUT_FORMAT_NONE) { opts->input_format = x; } else { error_log("Cannot use multiple input format options."); return -1; }
#define INPUT_SET_TYPE(x)          if (opts->input_type == OPTS_INPUT_TYPE_NONE) { opts->input_type = x; } else { error_log("Cannot use multiple input type options."); return -1; }
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

int opts_get(opts_p opts, int argc, char *argv[])
{
    int o;

    assert(opts);

    opts->input_format = OPTS_INPUT_FORMAT_NONE;
    opts->input_type = OPTS_INPUT_TYPE_NONE;
    opts->output_format = OPTS_OUTPUT_FORMAT_NONE;
    opts->output_type = OPTS_OUTPUT_TYPE_NONE;
    opts->compression = OPTS_OUTPUT_COMPRESSION_NONE;
    opts->network = OPTS_OUTPUT_NETWORK_NONE;
    opts->rehashes = OPTS_OUTPUT_REHASHES_NONE;
    opts->host_name = OPTS_HOST_NAME_NONE;
    opts->host_port = OPTS_HOST_PORT_NONE;
    opts->create = OPTS_CREATE_FALSE;
    opts->input_path = OPTS_INPUT_PATH_NONE;
    opts->output_path = OPTS_OUTPUT_PATH_NONE;

    // Turn off getopt errors. I print my own errors.
    opterr = 0;

    while ((o = getopt(argc, argv, "abjwhrsdbxvJAWHDMTPBCUR:n:p:c::F:f:")) != -1)
    {
        switch (o)
        {
            case 'a':
                INPUT_SET_FORMAT(OPTS_INPUT_FORMAT_ASCII);
                break;
            case 'b':
                INPUT_SET_FORMAT(OPTS_INPUT_FORMAT_BINARY);
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_BINARY);
                break;
            case 'j':
                INPUT_SET_FORMAT(OPTS_INPUT_FORMAT_JSON);
                break;

            case 'w':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_WIF);
                break;
            case 'h':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_HEX);
                break;
            case 'r':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_RAW);
                break;
            case 's':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_STRING);
                break;
            case 'd':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_DECIMAL);
                break;
            case 'x':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_SBD);
                break;
            case 'v':
                INPUT_SET_TYPE(OPTS_INPUT_TYPE_VANITY);
                break;

            case 'J':
                OUTPUT_SET_FORMAT(OPTS_OUTPUT_FORMAT_JSON);
                break;
            case 'A':
                OUTPUT_SET_FORMAT(OPTS_OUTPUT_FORMAT_ASCII);
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
                    error_log("Invalid command option or argument required: '-%c'.", optopt);
                }
                else
                {
                    error_log("Invalid command option character '\\x%x'.", optopt);
                }
                return -1;
        }
    }

    // Defalts
    if (opts->input_format == OPTS_INPUT_FORMAT_NONE)
    {
        opts->input_format = OPTS_INPUT_FORMAT_DEFAULT;
    }
    if (opts->input_type == OPTS_INPUT_TYPE_NONE)
    {
        opts->input_type = OPTS_INPUT_TYPE_DEFAULT;
    }
    if (opts->output_format == OPTS_OUTPUT_FORMAT_NONE)
    {
        opts->output_format = OPTS_OUTPUT_FORMAT_DEFAULT;
    }
    if (opts->output_type == OPTS_OUTPUT_TYPE_NONE)
    {
        opts->output_type = OPTS_OUTPUT_TYPE_DEFAULT;
    }
    if (opts->compression == OPTS_OUTPUT_COMPRESSION_NONE)
    {
        // Don't set a default for compression. Let each ctrl module decide its
        // own default behavior.
    }
    if (opts->network == OPTS_OUTPUT_NETWORK_NONE)
    {
        opts->network = OPTS_OUTPUT_NETWORK_DEFAULT;
    }

    return 1;
}