/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef OPTS_H
#define OPTS_H 1

#define OPTS_OUTPUT_NETWORK_NONE            0
#define OPTS_OUTPUT_NETWORK_MAINNET         1
#define OPTS_OUTPUT_NETWORK_TESTNET         2

#define OPTS_OUTPUT_REHASHES_NONE           NULL

#define OPTS_INPUT_PATH_NONE                NULL
#define OPTS_OUTPUT_PATH_NONE               NULL

#define OPTS_HOST_NAME_NONE                 NULL
#define OPTS_HOST_PORT_NONE                 0

#define OPTS_CREATE_FALSE                   0
#define OPTS_CREATE_TRUE                    1   // Optional arg. Can be used with privkey, database, and vanity

#define OPTS_SUBCOMMAND_NONE                NULL

#define OPTS_STRING_PRIVKEY                 "lbwhrsdxcLCUTHDR:"
#define OPTS_STRING_PUBKEY                  "lwhCU"
#define OPTS_STRING_ADDRESS                 "lwhvPB"
#define OPTS_STRING_NODE                    "n:p:T"
#define OPTS_STRING_UTXODB                  "f:"
#define OPTS_STRING_ADDRESSDB               "wscf:F:"
#define OPTS_STRING_HELP                    ""

typedef struct opts *opts_p;
struct opts {
    int input_format_list;
    int input_format_binary;
    int input_type_wif;
    int input_type_hex;
    int input_type_raw;
    int input_type_string;
    int input_type_decimal;
    int input_type_binary;
    int input_type_sbd;
    int input_type_vanity;
    int output_format_list;
    int output_format_binary;
    int output_type_hex;
    int output_type_decimal;
    int output_type_p2pkh;
    int output_type_p2wpkh;
    int compression_on;
    int compression_off;
    int network_test;

    char *rehashes;
    char *host_name;
    int host_port;
    int create;
    char *input_path;
    char *output_path;
    char *subcommand;
};

int opts_init(opts_p);
int opts_get(opts_p, int, char **, char *);

#endif
