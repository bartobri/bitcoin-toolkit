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
#include "mods/address.h"
#include "mods/opts.h"

static opts_p opts;

int btk_addressdb_main(void)
{
    int r;
    uint64_t sats = 0;
    char *input = NULL;
    char address[BUFSIZ];
    unsigned char tmp[BUFSIZ];

    UTXODBKey utxodb_key = NULL;
    UTXODBValue utxodb_value = NULL;

    r = opts_get(&opts);
    ERROR_CHECK_NEG(r, "Could not get command options.");

    if (opts->create)
    {
        r = addressdb_open(opts->output_path, opts->create);
        ERROR_CHECK_NEG(r, "Could not open address database.");

        utxodb_key = malloc(utxodb_sizeof_key());
        ERROR_CHECK_NULL(utxodb_key, "Memory Allocation Error.");

        utxodb_value = malloc(utxodb_sizeof_value());
        ERROR_CHECK_NULL(utxodb_value, "Memory Allocation Error.");

        r = utxodb_open(opts->input_path);
        ERROR_CHECK_NEG(r, "Could not open utxo database.");

        while ((r = utxodb_get(utxodb_key, utxodb_value, NULL)) == 1)
        {
            if (utxodb_value_has_address(utxodb_value))
            {
                memset(address, 0, BUFSIZ);
                sats = 0;

                r = utxodb_value_get_address(address, utxodb_value);
                ERROR_CHECK_NEG(r, "Can not get address from value.");

                r = addressdb_get(&sats, address);
                ERROR_CHECK_NEG(r, "Could not query address database.");

                sats += utxodb_value_get_amount(utxodb_value);

                if (sats > 0)
                {
                    r = addressdb_put(address, sats);
                    ERROR_CHECK_NEG(r, "Could not create new record for address database.");
                }

                printf("%s => %zu\n", address, sats);
            }
        }
        ERROR_CHECK_NEG(r, "Could not get record from utxo database.");

        utxodb_close();

        utxodb_value_free(utxodb_value);
        free(utxodb_key);
        free(utxodb_value);
    }
    else
    {
        r = addressdb_open(opts->input_path, opts->create);
        ERROR_CHECK_NEG(r, "Could not open address database.");

        r = input_get_str(&input, "Enter Input: ");
        ERROR_CHECK_NEG(r, "Could not get input.");

        if (opts->input_type == OPTS_INPUT_TYPE_WIF)
        {
            memset(address, 0, BUFSIZ);

            r = address_from_wif(address, input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");

            input = realloc(input, strlen(address) + 1);
            ERROR_CHECK_NULL(input, "Could not allocate memory.");

            strcpy(input, address);
        }
        else if (opts->input_type == OPTS_INPUT_TYPE_STRING)
        {
            memset(address, 0, BUFSIZ);

            r = address_from_str(address, input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");

            input = realloc(input, strlen(address) + 1);
            ERROR_CHECK_NULL(input, "Could not allocate memory.");

            strcpy(input, address);
        }

        r = base58check_decode(tmp, input, BASE58CHECK_TYPE_ADDRESS_MAINNET);
        ERROR_CHECK_NEG(r, "Error while decoding input.");

        r = addressdb_get(&sats, input);
        ERROR_CHECK_NEG(r, "Could not query address database.");

        printf("Address %s has balance: %"PRIu64"\n", input, sats);

        free(input);
    }

    addressdb_close();

    return EXIT_SUCCESS;
}