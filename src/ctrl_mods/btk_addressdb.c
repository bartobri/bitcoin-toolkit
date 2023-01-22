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
#include <assert.h>
#include "mods/error.h"
#include "mods/input.h"
#include "mods/addressdb.h"
#include "mods/utxodb.h"
#include "mods/base58check.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/opts.h"

static char *input_path    = OPTS_INPUT_PATH_NONE;
static char *output_path   = OPTS_OUTPUT_PATH_NONE;

int btk_addressdb_main(opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    uint64_t sats = 0;
    char address[BUFSIZ];
    unsigned char tmp[BUFSIZ];

    UTXODBKey utxodb_key = NULL;
    UTXODBValue utxodb_value = NULL;

    assert(opts);

    (void)input_len;

    // Override defaults
    if (opts->input_path) { input_path = opts->input_path; }
    if (opts->output_path) { output_path = opts->output_path; }

    if (opts->create)
    {
        r = addressdb_open(output_path, opts->create);
        ERROR_CHECK_NEG(r, "Could not open address database.");

        utxodb_key = malloc(utxodb_sizeof_key());
        ERROR_CHECK_NULL(utxodb_key, "Memory Allocation Error.");

        utxodb_value = malloc(utxodb_sizeof_value());
        ERROR_CHECK_NULL(utxodb_value, "Memory Allocation Error.");

        r = utxodb_open(input_path);
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
        r = addressdb_open(input_path, opts->create);
        ERROR_CHECK_NEG(r, "Could not open address database.");

        if (opts->input_type_wif)
        {
            memset(address, 0, BUFSIZ);

            r = address_from_wif(address, (char *)input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");

            input = realloc(input, strlen(address) + 1);
            ERROR_CHECK_NULL(input, "Could not allocate memory.");

            strcpy((char *)input, address);
        }
        else if (opts->input_type_string)
        {
            memset(address, 0, BUFSIZ);

            r = address_from_str(address, (char *)input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");

            input = realloc(input, strlen(address) + 1);
            ERROR_CHECK_NULL(input, "Could not allocate memory.");

            strcpy((char *)input, address);
        }

        r = base58check_decode(tmp, (char *)input, BASE58CHECK_TYPE_ADDRESS_MAINNET);
        ERROR_CHECK_NEG(r, "Error while decoding input.");

        r = addressdb_get(&sats, (char *)input);
        ERROR_CHECK_NEG(r, "Could not query address database.");

        printf("Address %s has balance: %"PRIu64"\n", (char *)input, sats);

        free(input);
    }

    addressdb_close();

    return EXIT_SUCCESS;
}

int btk_addressdb_requires_input(opts_p opts)
{
    assert(opts);

    if (opts->create)
    {
        return 0;
    }

    return 1;
}