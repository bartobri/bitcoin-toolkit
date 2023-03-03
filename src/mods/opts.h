/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef OPTS_H
#define OPTS_H 1

typedef struct opts *opts_p;
struct opts {
    int input_format_list;
    int input_format_binary;
    int input_format_json;
    int input_type_wif;
    int input_type_hex;
    int input_type_raw;
    int input_type_string;
    int input_type_decimal;
    int input_type_binary;
    int input_type_sbd;
    int output_format_list;
    int output_format_qrcode;
    int output_format_binary;
    int output_format_json;
    int output_type_wif;
    int output_type_hex;
    int output_type_decimal;
    int output_type_raw;
    int output_type_p2pkh;
    int output_type_p2wpkh;
    int output_stream;
    int stream_report;
    char *output_grep;
    int compression_on;
    int compression_off;
    int network_test;
    char *rehash;
    char *host_name;
    char *host_service;
    int create;
    int update;
    char *input_path;
    char *output_path;
    char *command;
    char *subcommand;
};

int opts_init(opts_p, char *);
int opts_get(opts_p, int, char **);

#endif
