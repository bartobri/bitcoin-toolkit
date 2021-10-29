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
#include "mods/pubkey.h"
#include "mods/hex.h"
#include "mods/script.h"

#define BTK_DATABASE_UTXO     1
#define BTK_DATABASE_ADDRESS  2

int btk_database_main(int argc, char *argv[])
{
    int o, r;
    int db_type = 0;
    int found = 0;
    size_t i;
    size_t input_raw_len = 0;
    size_t serialized_key_len = 0;
    size_t serialized_value_len = 0;
    size_t obfuscate_key_len = 0;
    size_t script_len = 0;
    char address[42];
    char *input = NULL;
    char *home_path = NULL;
    char *db_path = NULL;
    char *script = NULL;
    unsigned char *input_raw = NULL;
    unsigned char *serialized_key = NULL;
    unsigned char *serialized_value = NULL;
    unsigned char *obfuscate_key = NULL;
    unsigned char *tmp = NULL;

    DBRef utxo_ref;
    PubKey pubkey = NULL;
    UTXOKey key = NULL;
    UTXOValue value = NULL;

    while ((o = getopt(argc, argv, "aup:")) != -1)
    {
        switch (o)
        {
            case 'a':
                db_type = BTK_DATABASE_ADDRESS;
                break;
            case 'u':
                db_type = BTK_DATABASE_UTXO;
                break;
            case 'p':
                db_path = optarg;
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

    if (db_type == 0)
    {
        error_log("Missing database type argument.");
        return -1;
    }

    if (db_path == NULL)
    {
        home_path = getenv("HOME");
        if (home_path == NULL)
        {
            error_log("Unable to determine home directory.");
            return -1;
        }
        db_path = malloc(strlen(home_path) + strlen(UTXO_PATH) + 2);
        if (db_path == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }
        strcpy(db_path, home_path);
        strcat(db_path, "/");
        strcat(db_path, UTXO_PATH);
    }

    if (db_type == BTK_DATABASE_UTXO)
    {
        r = input_get_str(&input, "Enter TX Hash: ");
        if (r < 0)
        {
            error_log("Could not get input.");
            return -1;
        }

        r = database_open(&utxo_ref, db_path);
        if (r < 0)
        {
            error_log("Error while opening UTXO database.");
            return -1;
        }

        r = database_get(&obfuscate_key, &obfuscate_key_len, utxo_ref, (unsigned char *)UTXO_OFUSCATE_KEY_KEY, UTXO_OFUSCATE_KEY_KEY_LENGTH);
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

        r = database_iter_seek_key(utxo_ref, serialized_key, serialized_key_len);
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
                r = database_iter_get(&serialized_key, &serialized_key_len, utxo_ref, &serialized_value, &serialized_value_len);
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

                found = 1;

                printf("%-10"PRIu64" ", utxo_value_get_height(value));
                printf("%-10"PRIu64" ", utxo_key_get_vout(key));
                printf("%-10"PRIu64" ", utxo_value_get_amount(value));

                if (utxo_value_has_address(value))
                {
                    //TODO - 42 should be a defined var
                    memset(address, 0, 42);

                    r = utxo_value_get_address(address, value);
                    if (r < 0) {
                        error_log("Can not get address from value.");
                        return -1;
                    }

                    printf("Paid To: %s", address);
                }
                else if (utxo_value_has_compressed_pubkey(value) || utxo_value_has_uncompressed_pubkey(value))
                {
                    pubkey = malloc(pubkey_sizeof());
                    if (pubkey == NULL)
                    {
                        error_log("Memory allocation error.");
                        return -1;
                    }

                    free(tmp);

                    script_len = utxo_value_get_script_len(value);
                    tmp = malloc(script_len + 1);
                    if (tmp == NULL)
                    {
                        error_log("Memory allocation error.");
                        return -1;
                    }

                    tmp[0] = (unsigned char)utxo_value_get_n_size(value);

                    if (utxo_value_has_uncompressed_pubkey(value))
                    {
                        tmp[0] -= 2;
                    }

                    r = utxo_value_get_script(tmp + 1, value);
                    if (r < 0)
                    {
                        error_log("Can not get script from value.");
                        return -1;
                    }

                    r = pubkey_from_raw(pubkey, tmp, script_len + 1);
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

                    printf("Paid To: %s", address);

                    free(pubkey);
                }
                else
                {
                    free(tmp);

                    script_len = utxo_value_get_script_len(value);
                    tmp = malloc(script_len);
                    if (tmp == NULL)
                    {
                        error_log("Memory Allocation Error.");
                        return -1;
                    }

                    r = utxo_value_get_script(tmp, value);
                    if (r < 0)
                    {
                        error_log("Unable to get script from database value.");
                        return -1;
                    }

                    script = script_from_raw(tmp, script_len);

                    printf("%s", script);

                    free(script);
                }

                printf("\n");

                r = database_iter_next(utxo_ref);
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

            if (found == 0)
            {
                printf("No results found.\n");
            }
            
        }

        free(input_raw);
        free(key);
        free(value);
        free(serialized_key);
        free(input);
        // TODO - Do not free db_path when assigned from optarg.
        // Commenting out for now.
        //free(db_path);

        database_close(utxo_ref);
    }
    else if (db_type == BTK_DATABASE_ADDRESS)
    {
        printf("Operating on address database\n");
    }

    return EXIT_SUCCESS;
}