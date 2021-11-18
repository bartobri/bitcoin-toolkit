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
#include "mods/error.h"
#include "mods/input.h"
#include "mods/hex.h"
#include "mods/pubkey.h"
#include "mods/script.h"
#include "mods/utxodb.h"

#define BTK_UTXODB_TX_LENGTH          32
#define BTK_UTXODB_MAX_ADDRESS_LENGTH 42
#define BTK_UTXODB_MAX_SCRIPT_LENGTH  1000

int btk_utxodb_main(int argc, char *argv[])
{
    int o, r;
    size_t script_len;
    char script_readable[BTK_UTXODB_MAX_SCRIPT_LENGTH * 4];
    char *input = NULL;
    char *db_path = NULL;
    char *tmp = NULL;
    unsigned char input_raw[BTK_UTXODB_TX_LENGTH];
    unsigned char script[BTK_UTXODB_MAX_SCRIPT_LENGTH];
    UTXODBKey key = NULL;
    UTXODBValue value = NULL;
    PubKey pubkey = NULL;

    while ((o = getopt(argc, argv, "p:")) != -1)
    {
        switch (o)
        {
            case 'p':
                db_path = optarg;
                break;
            case '?':
                error_log("See 'btk help database' to read about available argument options.");
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

    r = input_get_str(&input, "Enter TX Hash: ");
    if (r < 0)
    {
        error_log("Could not get input.");
        return -1;
    }

    if (strlen(input) != (UTXODB_TX_HASH_LENGTH * 2))
    {
        error_log("Input must be a %i byte hexidecimal string.", (UTXODB_TX_HASH_LENGTH * 2));
        return -1;
    }

    r = hex_str_to_raw(input_raw, input);
    if (r < 0)
    {
        error_log("Can not convert hex input to raw binary.");
        return -1;
    }

    key = malloc(utxodb_sizeof_key());
    value = malloc(utxodb_sizeof_value());
    pubkey = malloc(pubkey_sizeof());
    if (key == NULL || value == NULL || pubkey == NULL)
    {
        error_log("Memory Allocation Error.");
        return -1;
    }

    r = utxodb_open(db_path);
    if (r < 0)
    {
        error_log("Could not open utxo database.");
        return -1;
    }

    while ((r = utxodb_get(key, value, input_raw)) == 1)
    {
        printf("%"PRIu64",", utxodb_value_get_height(value));
        printf("%"PRIu64",", utxodb_key_get_vout(key));
        printf("%"PRIu64",", utxodb_value_get_amount(value));

        memset(script, 0, BTK_UTXODB_MAX_SCRIPT_LENGTH);
        memset(script_readable, 0, BTK_UTXODB_MAX_SCRIPT_LENGTH * 4);

        script_len = utxodb_value_get_script_len(value);

        if (utxodb_value_has_address(value))
        {
            r = utxodb_value_get_address(script_readable, value);
            if (r < 0)
            {
                error_log("Can not get address from value.");
                return -1;
            }
        }
        else if (utxodb_value_has_compressed_pubkey(value) || utxodb_value_has_uncompressed_pubkey(value))
        {
            script[0] = (unsigned char)utxodb_value_get_n_size(value);
            if (utxodb_value_has_uncompressed_pubkey(value))
            {
                script[0] -= 2;
            }

            r = utxodb_value_get_script(script + 1, value);
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

            if (utxodb_value_has_uncompressed_pubkey(value))
            {
                pubkey_decompress(pubkey);
                if (r < 0)
                {
                    error_log("Can not decompress pubkey.");
                    return -1;
                }
            }

            r = pubkey_to_address(script_readable, pubkey);
            if (r < 0)
            {
                error_log("Can not get address from pubkey.");
                return -1;
            }

        }
        else
        {
            r = utxodb_value_get_script(script, value);
            if (r < 0)
            {
                error_log("Unable to get script from database value.");
                return -1;
            }

            // TODO - Modify this to return int and take output as param
            tmp = script_from_raw(script, script_len);
            if (tmp == NULL)
            {
                error_log("Could not translate script from raw data.");
                return -1;
            }

            strcpy(script_readable, tmp);

            free(tmp);
            tmp = NULL;
        }

        printf("%s", script_readable);
        printf("\n");
    }
    if (r < 0)
    {
        error_log("Could not open utxo database.");
        return -1;
    }

    utxodb_close();

    utxodb_value_free(value);
    free(key);
    free(value);
    free(pubkey);

    return EXIT_SUCCESS;
}