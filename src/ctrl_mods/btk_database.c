/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "mods/input.h"
#include "mods/error.h"
#include "mods/database.h"
#include "mods/databases/utxo.h"
#include "mods/hex.h"

int btk_database_main(int argc, char *argv[])
{
    int o, r;
    size_t i;
    size_t input_raw_len = 0;
    size_t serialized_key_len = 0;
    size_t serialized_value_len = 0;
    size_t obfuscate_key_len = 0;
    char *db_type = NULL;
    char *input = NULL;
    unsigned char *input_raw = NULL;
    unsigned char *serialized_key = NULL;
    unsigned char *serialized_value = NULL;
    unsigned char *obfuscate_key = NULL;
    unsigned char *tmp = NULL;

    UTXOKey key = NULL;
    UTXOValue value = NULL;

    while ((o = getopt(argc, argv, "t:")) != -1)
    {
        switch (o)
        {
            case 't':
                db_type = optarg;
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

    if (db_type == NULL)
    {
        error_log("Missing required argument: -t <database type>.");
        return -1;
    }

    if (strcmp(db_type, "utxo") == 0)
    {
        r = input_get_str(&input, "Enter TX Hash: ");
        if (r < 0)
        {
            error_log("Could not get input.");
            return -1;
        }

        r = database_open(UTXO_DATABASE);
        if (r < 0)
        {
            error_log("Error while opening UTXO database.");
            return -1;
        }

        r = database_get(&obfuscate_key, &obfuscate_key_len, (unsigned char *)UTXO_OFUSCATE_KEY_KEY, UTXO_OFUSCATE_KEY_KEY_LENGTH);
        if (r < 0 || obfuscate_key == NULL || obfuscate_key_len == 0) {
            error_log("Can not get obfuscate key.");
            return -1;
        }

        // Drop first char
        obfuscate_key++;
        obfuscate_key_len -= 1;

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

        input_raw_len = strlen(input) / 2;
        input_raw = malloc(input_raw_len);
        if (input_raw == NULL)
        {
            error_log("Memory Allocation Error");
            return -1;
        }

        r = hex_str_to_raw(input_raw, input);
        if (r < 0)
        {
            error_log("Can not convert hex input to raw binary.");
            return -1;
        }

        r = utxo_set_key(key, input_raw, 0);
        if (r < 0)
        {
            error_log("Can not set UTXO key values.");
            return -1;
        }

        serialized_key = malloc(UTXO_KEY_MAX_LENGTH);
        if (serialized_key == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }

        r = utxo_serialize_key(serialized_key, &serialized_key_len, key);
        if (r < 0 || serialized_key_len == 0) {
            error_log("Unable to serialize UTXO key.");
            return -1;
        }

        r = database_iter_seek_key(serialized_key, serialized_key_len);
        if (r < 0) {
            error_log("Could not seek database iterator.");
            return -1;
        }
        else if (r == 0)
        {
            // End of database.
        }
        else
        {
            do
            {
                r = database_iter_get(&serialized_key, &serialized_key_len, &serialized_value, &serialized_value_len);
                if (r < 0)
                {
                    error_log("Could not get data from database.");
                    return -1;
                }

                // De-obfuscate the value
                for (i = 0; i < serialized_value_len; i++)
                {
                    serialized_value[i] ^= obfuscate_key[i % obfuscate_key_len];
                }

                r = utxo_set_key_from_raw(key, serialized_key, serialized_key_len);
                if (r < 0)
                {
                    error_log("Could not deserialize raw key.");
                    return -1;
                }

                r = utxo_set_value_from_raw(value, serialized_value, serialized_value_len);
                if (r < 0)
                {
                    error_log("Could not deserialize raw value.");
                    return -1;
                }

                tmp = malloc(UTXO_TX_HASH_LENGTH);
                if (tmp == NULL)
                {
                    error_log("Memory Allocation Error.");
                    return -1;
                }

                r = utxo_key_get_tx_hash(tmp, key);
                if (r < 0)
                {
                    error_log("Unable to get TX hash from key.");
                    return -1;
                }

                if (memcmp(input_raw, tmp, UTXO_TX_HASH_LENGTH) != 0)
                {
                    break;
                }

                printf("Vout: %"PRIu64", ", utxo_key_get_vout(key));
                printf("Height: %"PRIu64", ", utxo_value_get_height(value));
                printf("Amount: %"PRIu64"\n", utxo_value_get_amount(value));

                r = database_iter_next();
                if (r < 0)
                {
                    error_log("Unable to set database iterator to next key.");
                    return -1;
                }
                else if (r == 0)
                {
                    // End of database.
                    break;
                }

                free(tmp);
                free(serialized_value);
            }
            while (1);
            
        }

        free(input_raw);
        free(key);
        free(value);
        free(serialized_key);
        free(input);

        database_close();
    }
    else
    {
        error_log("Unknown database type specified: %s.", db_type);
        return -1;
    }

    return EXIT_SUCCESS;
}