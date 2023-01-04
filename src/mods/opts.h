/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef OPTS_H
#define OPTS_H 1

#define OPTS_INPUT_FORMAT_NONE              0
#define OPTS_INPUT_FORMAT_ASCII             1
#define OPTS_INPUT_FORMAT_BINARY            2
#define OPTS_INPUT_FORMAT_JSON              3
#define OPTS_INPUT_FORMAT_DEFAULT           3 //OPTS_INPUT_FORMAT_JSON

#define OPTS_INPUT_TYPE_NONE                0
#define OPTS_INPUT_TYPE_WIF                 1
#define OPTS_INPUT_TYPE_HEX                 2
#define OPTS_INPUT_TYPE_RAW                 3
#define OPTS_INPUT_TYPE_STRING              4
#define OPTS_INPUT_TYPE_DECIMAL             5
#define OPTS_INPUT_TYPE_BINARY              6
#define OPTS_INPUT_TYPE_SBD                 7
#define OPTS_INPUT_TYPE_VANITY              8
#define OPTS_INPUT_TYPE_GUESS               9
#define OPTS_INPUT_TYPE_DEFAULT             OPTS_INPUT_TYPE_GUESS

#define OPTS_OUTPUT_FORMAT_NONE             0
#define OPTS_OUTPUT_FORMAT_ASCII            1
#define OPTS_OUTPUT_FORMAT_JSON             2
#define OPTS_OUTPUT_FORMAT_DEFAULT          OPTS_OUTPUT_FORMAT_JSON

#define OPTS_OUTPUT_TYPE_NONE               0
#define OPTS_OUTPUT_TYPE_WIF                1
#define OPTS_OUTPUT_TYPE_HEX                2
#define OPTS_OUTPUT_TYPE_DECIMAL            3
#define OPTS_OUTPUT_TYPE_P2PKH              4    // Legacy address
#define OPTS_OUTPUT_TYPE_P2WPKH             5    // Bech32 address
#define OPTS_OUTPUT_TYPE_DEFAULT            OPTS_OUTPUT_TYPE_HEX

#define OPTS_OUTPUT_COMPRESSION_NONE        0
#define OPTS_OUTPUT_COMPRESSION_TRUE        1
#define OPTS_OUTPUT_COMPRESSION_FALSE       2
#define OPTS_OUTPUT_COMPRESSION_BOTH        3
#define OPTS_OUTPUT_COMPRESSION_DEFAULT     OPTS_OUTPUT_COMPRESSION_TRUE

#define OPTS_OUTPUT_NETWORK_NONE            0
#define OPTS_OUTPUT_NETWORK_MAINNET         1
#define OPTS_OUTPUT_NETWORK_TESTNET         2
#define OPTS_OUTPUT_NETWORK_DEFAULT         OPTS_OUTPUT_NETWORK_MAINNET

#define OPTS_OUTPUT_REHASHES_NONE           NULL

#define OPTS_INPUT_PATH_NONE                NULL
#define OPTS_OUTPUT_PATH_NONE               NULL

#define OPTS_HOST_NAME_NONE                 NULL
#define OPTS_HOST_PORT_NONE                 0

#define OPTS_CREATE_FALSE                   0
#define OPTS_CREATE_TRUE                    1   // Optional arg. Can be used with privey, database, and vanity

typedef struct opts *opts_p;
struct opts {
    int input_format;
    int input_type;
    int output_format;
    int output_type;
    int compression;
    int network;
    char *rehashes;
    char *host_name;
    int host_port;
    int create;
    char *input_path;
    char *output_path;
};

int opts_init(int, char **);
int opts_get(opts_p *);

#endif
