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

int btk_database_main(int argc, char *argv[])
{
    int o, r;
    size_t i, j;
    size_t serialized_key_len = 0;
    size_t serialized_value_len = 0;
    size_t obfuscate_key_len = 0;
    char *db_type = NULL;
    char *input = NULL;
    unsigned char *serialized_key = NULL;
    unsigned char *serialized_value = NULL;
    unsigned char *obfuscate_key = NULL;

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

        for (i = 0; i < 1000; i++)
        {
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

            r = utxo_set_key_from_hex(key, input, i);
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

            r = database_get(&serialized_value, &serialized_value_len, serialized_key, serialized_key_len);
            if (r < 0) {
                error_log("Unable to get value from database.");
                return -1;
            }

            if (serialized_value_len)
            {
                // De-obfuscate the value
                for (j = 0; j < serialized_value_len; j++)
                {
                    serialized_value[j] ^= obfuscate_key[j % obfuscate_key_len];
                }

                r = utxo_set_value_from_raw(value, serialized_value, serialized_value_len);
                if (r < 0)
                {
                    error_log("Could not deserialize raw value.");
                    return -1;
                }

                printf("Vout: %zu, ", i);
                printf("Height: %"PRIu64", ", utxo_value_get_height(value));
                printf("Amount: %"PRIu64"\n", utxo_value_get_amount(value));

                free(serialized_value);
                
            }
            else
            {
                //printf("Key does not exist.\n");
            }

            free(key);
            free(value);
            free(serialized_key);
        }

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