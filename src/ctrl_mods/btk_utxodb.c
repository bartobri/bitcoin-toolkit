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
#define BTK_UTXODB_MAX_SCRIPT_LENGTH  100

static char *db_path = NULL;

int btk_utxodb_init(int argc, char *argv[])
{
    int o;
    char *command = NULL;

    command = argv[1];

    while ((o = getopt(argc, argv, "p:")) != -1)
    {
        switch (o)
        {
            case 'p':
                db_path = optarg;
                break;
            case '?':
                error_log("See 'btk help %s' to read about available argument options.", command);
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

    return 1;
}

int btk_utxodb_main(void)
{
    int r;
    char address[BTK_UTXODB_MAX_ADDRESS_LENGTH];
    char *input = NULL;
    unsigned char input_raw[BTK_UTXODB_TX_LENGTH];
    UTXODBKey key = NULL;
    UTXODBValue value = NULL;

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
    if (key == NULL || value == NULL)
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

        if (utxodb_value_has_address(value))
        {
            r = utxodb_value_get_address(address, value);
            if (r < 0)
            {
                error_log("Can not get address from value.");
                return -1;
            }
            else if (r > 0)
            {
                printf("%s", address);
            }
        }

        printf("\n");
    }
    if (r < 0)
    {
        error_log("Could not get record from utxo database.");
        return -1;
    }

    utxodb_close();

    utxodb_value_free(value);
    free(key);
    free(value);

    return EXIT_SUCCESS;
}

int btk_utxodb_cleanup(void)
{
    return 1;
}