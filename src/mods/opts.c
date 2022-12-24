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

#define INPUT_FORMAT_NONE              0
#define INPUT_FORMAT_ASCII             1
#define INPUT_FORMAT_BINARY            2
#define INPUT_FORMAT_JSON              3
#define INPUT_FORMAT_DEFAULT           INPUT_FORMAT_JSON

#define INPUT_TYPE_NONE                0
#define INPUT_TYPE_WIF                 1
#define INPUT_TYPE_HEX                 2
#define INPUT_TYPE_RAW                 3
#define INPUT_TYPE_STRING              4
#define INPUT_TYPE_DECIMAL             5
#define INPUT_TYPE_BINARY              6
#define INPUT_TYPE_SBD                 7
#define INPUT_TYPE_GUESS               8
#define INPUT_TYPE_DEFAULT             INPUT_TYPE_GUESS

#define OUTPUT_FORMAT_NONE             0
#define OUTPUT_FORMAT_ASCII            1
#define OUTPUT_FORMAT_JSON             2
#define OUTPUT_FORMAT_DEFAULT          OUTPUT_FORMAT_JSON

#define OUTPUT_TYPE_NONE               0
#define OUTPUT_TYPE_WIF                1
#define OUTPUT_TYPE_HEX                2
#define OUTPUT_TYPE_DECIMAL            3
#define OUTPUT_TYPE_P2PKH              4    // Legacy address
#define OUTPUT_TYPE_P2WPKH             5    // Bech32 address
#define OUTPUT_TYPE_DEFAULT            OUTPUT_TYPE_HEX

#define OUTPUT_COMPRESSION_NONE        0
#define OUTPUT_COMPRESSION_TRUE        1
#define OUTPUT_COMPRESSION_FALSE       2
#define OUTPUT_COMPRESSION_BOTH        3
#define OUTPUT_COMPRESSION_DEFAULT     OUTPUT_COMPRESSION_TRUE

#define OUTPUT_NETWORK_NONE            0
#define OUTPUT_NETWORK_MAINNET         1
#define OUTPUT_NETWORK_TESTNET         2
#define OUTPUT_NETWORK_DEFAULT         OUTPUT_NETWORK_MAINNET

#define OUTPUT_REHASHES_NONE           NULL

#define INPUT_PATH_NONE                NULL
#define OUTPUT_PATH_NONE               NULL

#define HOST_NAME_NONE                 NULL
#define HOST_PORT_NONE                 0

#define CREATE_FALSE                   0
#define CREATE_TRUE                    1   // Optional arg. Can be used with privey, database, and vanity

#define INPUT_SET_FORMAT(x)        if (input_format == INPUT_FORMAT_NONE) { input_format = x; } else { error_log("Cannot use multiple input format options."); return -1; }
#define INPUT_SET_TYPE(x)          if (input_type == INPUT_TYPE_NONE) { input_type = x; } else { error_log("Cannot use multiple input type options."); return -1; }
#define OUTPUT_SET_FORMAT(x)       if (output_format == OUTPUT_FORMAT_NONE) { output_format = x; } else { error_log("Cannot use multiple output format options."); return -1; }
#define OUTPUT_SET_TYPE(x)         if (output_type == OUTPUT_TYPE_NONE) { output_type = x; } else { error_log("Cannot use multiple output type options."); return -1; }
#define OUTPUT_SET_COMPRESSION(x)  if (compression == OUTPUT_COMPRESSION_NONE) { compression = x; } else { compression = OUTPUT_COMPRESSION_BOTH; }
#define OUTPUT_SET_NETWORK(x)      if (network == OUTPUT_NETWORK_NONE) { network = x; } else { error_log("Cannot use multiple network options."); return -1; }
#define OUTPUT_SET_REHASHES(x)     if (rehashes == OUTPUT_REHASHES_NONE) { rehashes = x; } else { error_log("Cannot use multiple rehash options."); return -1; }
#define HOST_SET_NAME(x)           if (host_name == HOST_NAME_NONE) { host_name = x; } else { error_log("Cannot use multiple host name options."); return -1; }
#define HOST_SET_PORT(x)           if (host_port == HOST_PORT_NONE) { host_port = x; } else { error_log("Cannot use multiple host port options."); return -1; }
#define CREATE_SET_CREATE(x)       if (create == CREATE_FALSE) { create = x; } else { error_log("Cannot use multiple create options."); return -1; }
#define INPUT_SET_PATH(x)          if (input_path == INPUT_PATH_NONE) { input_path = x; } else { error_log("Cannot use multiple input path options."); return -1; }
#define OUTPUT_SET_PATH(x)         if (output_path == OUTPUT_PATH_NONE) { output_path = x; } else { error_log("Cannot use multiple input path options."); return -1; }

static int input_format = INPUT_FORMAT_NONE;
static int input_type = INPUT_TYPE_NONE;
static int output_format = OUTPUT_FORMAT_NONE;
static int output_type = OUTPUT_TYPE_NONE;
static int compression = OUTPUT_COMPRESSION_NONE;
static int network = OUTPUT_NETWORK_NONE;
static char *rehashes = OUTPUT_REHASHES_NONE;
static char *host_name = HOST_NAME_NONE;
static int host_port = HOST_PORT_NONE;
static int create = CREATE_FALSE;
static char *input_path = INPUT_PATH_NONE;
static char *output_path = OUTPUT_PATH_NONE;

int opts_init(int argc, char *argv[])
{
    int o;

    while ((o = getopt(argc, argv, "abjwhrsdbxJAWHDMTPBCUR:n:p:c::F:f:")) != -1)
    {
        switch (o)
        {
            case 'a':
                INPUT_SET_FORMAT(INPUT_FORMAT_ASCII);
                break;
            case 'b':
                INPUT_SET_FORMAT(INPUT_FORMAT_BINARY);
                INPUT_SET_TYPE(INPUT_TYPE_BINARY);
                break;
            case 'j':
                INPUT_SET_FORMAT(INPUT_FORMAT_JSON);
                break;

            case 'w':
                INPUT_SET_TYPE(INPUT_TYPE_WIF);
                break;
            case 'h':
                INPUT_SET_TYPE(INPUT_TYPE_HEX);
                break;
            case 'r':
                INPUT_SET_TYPE(INPUT_TYPE_RAW);
                break;
            case 's':
                INPUT_SET_TYPE(INPUT_TYPE_STRING);
                break;
            case 'd':
                INPUT_SET_TYPE(INPUT_TYPE_DECIMAL);
                break;
            case 'x':
                INPUT_SET_TYPE(INPUT_TYPE_SBD);
                break;

            case 'J':
                OUTPUT_SET_FORMAT(OUTPUT_FORMAT_JSON);
                break;
            case 'A':
                OUTPUT_SET_FORMAT(OUTPUT_FORMAT_ASCII);
                break;

            case 'W':
                OUTPUT_SET_TYPE(OUTPUT_TYPE_WIF);
                break;
            case 'H':
                OUTPUT_SET_TYPE(OUTPUT_TYPE_HEX);
                break;
            case 'D':
                OUTPUT_SET_TYPE(OUTPUT_TYPE_DECIMAL);
                break;
            case 'P':
                OUTPUT_SET_TYPE(OUTPUT_TYPE_P2PKH);
                break;
            case 'B':
                OUTPUT_SET_TYPE(OUTPUT_TYPE_P2WPKH);
                break;

            case 'C':
                OUTPUT_SET_COMPRESSION(OUTPUT_COMPRESSION_TRUE);
                break;
            case 'U':
                OUTPUT_SET_COMPRESSION(OUTPUT_COMPRESSION_FALSE);
                break;

            case 'M':
                OUTPUT_SET_NETWORK(OUTPUT_NETWORK_MAINNET);
                break;
            case 'T':
                OUTPUT_SET_NETWORK(OUTPUT_NETWORK_TESTNET);
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
                CREATE_SET_CREATE(CREATE_TRUE);
                break;

            case 'f':
                INPUT_SET_PATH(optarg);
                break;
            case 'F':
                OUTPUT_SET_PATH(optarg);
                break;

            case '?':
                error_log("See 'btk help %s' to read about available argument options.", argv[1]);
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
    if (input_format == INPUT_FORMAT_NONE)
    {
        input_format = INPUT_FORMAT_DEFAULT;
    }
    if (input_type == INPUT_TYPE_NONE)
    {
        input_type = INPUT_TYPE_DEFAULT;
    }
    if (output_format == OUTPUT_FORMAT_NONE)
    {
        output_format = OUTPUT_FORMAT_DEFAULT;
    }
    if (output_type == OUTPUT_TYPE_NONE)
    {
        output_type = OUTPUT_TYPE_DEFAULT;
    }
    if (compression == OUTPUT_COMPRESSION_NONE)
    {
        compression = OUTPUT_COMPRESSION_DEFAULT;
    }
    if (network == OUTPUT_NETWORK_NONE)
    {
        network = OUTPUT_NETWORK_DEFAULT;
    }

    return 1;
}
