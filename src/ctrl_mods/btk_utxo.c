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
#include "mods/pubkey.h"
#include "mods/hex.h"
#include "mods/input.h"
#include "mods/error.h"

int btk_utxo_main(int argc, char *argv[])
{
    int i, o, r;
    char *input_tx = NULL;
    int vout = 0;
    UTXOKey key = NULL;
    UTXOValue value = NULL;

    // For testing
    char address[21 * 2];
    uint64_t amount = 0;

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
        if (key == NULL || value == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }

        r = utxo_open_database(UTXO_DATABASE);
        if (r < 0)
        {
            error_log("Database open error.");
            return -1;
        }

        r = utxo_database_iter_seek_start();
        if (r < 0)
        {
            error_log("Can not seek database iterator to start.");
            return -1;
        }

        for (i = 0; i < 1000000; i++)
        {
            r = utxo_database_iter_get_next(key, value);
            if (r < 0)
            {
                error_log("Can not get next utxo record.");
                return -1;
            }

            if (utxo_value_has_address(value))
            {
                r = utxo_value_get_address(address, value);
                if (r < 0) {
                    error_log("Can not get address from value.");
                    return -1;
                }

                amount = utxo_value_get_amount(value);

                printf("%i Address %s has %lu satoshis\n\n", i, address, amount);
            }
            else if (utxo_value_has_compressed_pubkey(value) || utxo_value_has_uncompressed_pubkey(value))
            {
                PubKey pubkey;
                unsigned char* script;
                size_t script_len;

                script = NULL;
                pubkey = NULL;

                pubkey = malloc(pubkey_sizeof());
                if (pubkey == NULL)
                {
                    error_log("Memory allocation error.");
                    return -1;
                }

                script_len = utxo_value_get_script_len(value);

                script = malloc(script_len + 1);
                if (script == NULL)
                {
                    error_log("Memory allocation error.");
                    return -1;
                }

                script[0] = (unsigned char)utxo_value_get_n_size(value);

                if (utxo_value_has_uncompressed_pubkey(value))
                {
                    script[0] -= 2;
                }

                r = utxo_value_get_script(script + 1, value);
                if (r < 0)
                {
                    error_log("Can not get script from value.");
                    return -1;
                }

                r = pubkey_from_raw(pubkey, script, script_len + 1);
                if (r < 0)
                {
                    error_log("Can not get pubkey object from script.");
                    return -1;
                }

                if (utxo_value_has_uncompressed_pubkey(value))
                {
                    r = pubkey_decompress(pubkey);
                    if (r < 0)
                    {
                        error_log("Can not decompress pubkey.");
                        return -1;
                    }

                }

                r = pubkey_to_address(address, pubkey);
                if (r < 0)
                {
                    error_log("Can not get address from pubkey.");
                    return -1;
                }

                amount = utxo_value_get_amount(value);

                printf("%i Address %s has %lu satoshis\n\n", i, address, amount);

                free(script);
                free(pubkey);
            }

            utxo_value_free(value);
        }

        utxo_close_database();

        free(key);
        free(value);
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

            amount = utxo_value_get_amount(value);

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
