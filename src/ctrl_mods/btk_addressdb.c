/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include "mods/error.h"
#include "mods/input.h"
#include "mods/addressdb.h"
#include "mods/utxodb.h"
#include "mods/base58check.h"
#include "mods/pubkey.h"

#define BTK_ADDRESSDB_MAX_ADDRESS_LENGTH   42
#define BTK_ADDRESSDB_INPUT_ADDRESS        1
#define BTK_ADDRESSDB_INPUT_PRIVKEY_WIF    2

int btk_addressdb_main(int argc, char *argv[])
{
    int o, r;
    int create = false;
    int input_mode = BTK_ADDRESSDB_INPUT_ADDRESS;
    uint64_t sats = 0;
    char *input = NULL;
    char *db_path = NULL;
    char *utxodb_path = NULL;
    char address[BTK_ADDRESSDB_MAX_ADDRESS_LENGTH];
    unsigned char tmp[BUFSIZ];

    UTXODBKey utxodb_key = NULL;
    UTXODBValue utxodb_value = NULL;

    while ((o = getopt(argc, argv, "p:u:cr")) != -1)
    {
        switch (o)
        {
            case 'p':
                db_path = optarg;
                break;
            case 'u':
                utxodb_path = optarg;
                break;
            case 'c':
                create = true;
                break;
            case 'r':
                input_mode = BTK_ADDRESSDB_INPUT_PRIVKEY_WIF;
                break;
            case '?':
                error_log("See 'btk help addressdb' to read about available argument options.");
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

    if (create == true)
    {
        utxodb_key = malloc(utxodb_sizeof_key());
        utxodb_value = malloc(utxodb_sizeof_value());
        if (utxodb_key == NULL || utxodb_value == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }

        r = utxodb_open(utxodb_path);
        if (r < 0)
        {
            error_log("Could not open utxo database.");
            return -1;
        }

        r = addressdb_open(db_path, create);
        if (r < 0)
        {
            error_log("Could not open address database.");
            return -1;
        }

        while ((r = utxodb_get(utxodb_key, utxodb_value, NULL)) == 1)
        {
            if (utxodb_value_has_address(utxodb_value))
            {
                memset(address, 0, BTK_ADDRESSDB_MAX_ADDRESS_LENGTH);
                sats = 0;

                r = utxodb_value_get_address(address, utxodb_value);
                if (r < 0)
                {
                    error_log("Can not get address from value.");
                    return -1;
                }

                r = addressdb_get(&sats, address);
                if (r < 0)
                {
                    error_log("Could not open address database.");
                    return -1;
                }

                sats += utxodb_value_get_amount(utxodb_value);

                if (sats > 0)
                {
                    r = addressdb_put(address, sats);
                    if (r < 0)
                    {
                        error_log("Could not create new record for address database.");
                        return -1;
                    }
                }

                printf("%s => %zu\n", address, sats);
            }
        }
        if (r < 0)
        {
            error_log("Could not get record from utxo database.");
            return -1;
        }

        addressdb_close();
        utxodb_close();
    }
    else
    {
        r = input_get_str(&input, "Enter Input: ");
        if (r < 0)
        {
            error_log("Could not get input.");
            return -1;
        }

        if (input_mode == BTK_ADDRESSDB_INPUT_PRIVKEY_WIF)
        {
            memset(address, 0, BTK_ADDRESSDB_MAX_ADDRESS_LENGTH);

            r = pubkey_address_from_wif(address, input);
            if (r < 0)
            {
                error_log("Could not calculate address from private key.");
                return -1;
            }

            input = realloc(input, strlen(address) + 1);
            if (r < 0)
            {
                error_log("Could not allocate memory.");
                return -1;
            }

            strcpy(input, address);
        }

        r = base58check_decode(tmp, input, BASE58CHECK_TYPE_ADDRESS_MAINNET);
        if (r < 0)
        {
            error_log("Error while decoding input.");
            return -1;
        }

        r = addressdb_open(db_path, false);
        if (r < 0)
        {
            error_log("Could not open address database.");
            return -1;
        }

        r = addressdb_get(&sats, input);
        if (r < 0)
        {
            error_log("Could not open address database.");
            return -1;
        }

        printf("Address %s has balance: %"PRIu64"\n", input, sats);

        addressdb_close();

        free(input);
    }

    return EXIT_SUCCESS;
}