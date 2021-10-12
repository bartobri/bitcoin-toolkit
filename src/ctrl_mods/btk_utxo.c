/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mods/databases/utxo.h"
#include "mods/hex.h"
#include "mods/input.h"
#include "mods/error.h"

int btk_utxo_main(int argc, char *argv[])
{
    int o, r;
    char *input_tx = NULL;
    int vout = 0;
    UTXOKey key = NULL;
    UTXOValue value = NULL;

    char *build_location = NULL;

    while ((o = getopt(argc, argv, "v:b:")) != -1)
    {
        switch (o)
        {
            case 'v':
                vout = atoi(optarg);
                break;
            case 'b':
                build_location = optarg;
                break;
            case '?':
                error_log("See 'btk help %s' to read about available argument options.", argv[1]);
                if (isprint(optopt))
                {
                    error_log("Invalid command option '-%c'.", optopt);
                }
                else
                {
                    error_log("Invalid command option character '\\x%x'.", optopt);
                }
                return -1;
        }
    }

    if (build_location != NULL)
    {
        key = malloc(utxo_sizeof_key());
        value = malloc(utxo_sizeof_value());
    }
    else
    {
        r = input_get_str(&input_tx, "Enter TX Hash: ");
        if (r < 0)
        {
            error_log("Could not get input.");
            return -1;
        }

        printf("Allocating memory for structs...\n");
        key = malloc(utxo_sizeof_key());
        if (key == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }

        value = malloc(utxo_sizeof_value());
        if (value == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }

        printf("Setting key...\n");
        r = utxo_set_key_from_hex(key, input_tx, vout);
        if (r < 0)
        {
            error_log("Can not set UTXO key values.");
            return -1;
        }

        printf("Opening database...\n");
        r = utxo_open_database(UTXO_DATABASE);
        if (r < 0)
        {
            error_log("Database open error.");
            return -1;
        }

        printf("Getting Key Value...\n");
        r = utxo_get(value, key);
        if (r < 0) {
            error_log("Can not get key value.");
            return -1;
        }

        printf("Can we dump an address? ");
        if (utxo_value_has_address(value))
        {
            printf("Yes\n");

            char address[21 * 2];
            uint64_t amount = 0;

            r = utxo_value_get_address(address, value);
            if (r < 0) {
                error_log("Can not get address from value.");
                return -1;
            }

            r = utxo_value_get_amount(&amount, value);
            if (r < 0) {
                error_log("Can not get address from value.");
                return -1;
            }

            printf("Address %s has %lu satoshis\n", address, amount);
        }
        else
        {
            printf("No\n");
        }

        printf("Closing database...\n");
        utxo_close_database();

        printf("Destroying Key and Value...\n");
        free(key);
        utxo_value_free(value);
        free(value);
    }

    return EXIT_SUCCESS;
}
