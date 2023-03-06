/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "mods/error.h"
#include "mods/database.h"
#include "mods/serialize.h"

#define BALANCE_DEFAULT_PATH             ".btk/balance"

static DBRef dbref = NULL;

int balance_open(char *path, bool create)
{
    int r;
    
    if (path == NULL)
    {
        path = malloc(BUFSIZ);
        ERROR_CHECK_NULL(path, "Memory allocation error.");

        strcpy(path, getenv("HOME"));
        if (*path == 0)
        {
            error_log("Unable to determine home directory.");
            return -1;
        }
        strcat(path, "/");
        strcat(path, BALANCE_DEFAULT_PATH);
    }

    r = database_open(&dbref, path, create);
    ERROR_CHECK_NEG(r, "Could not open the database.");

    return 1;
}

void balance_close(void)
{
    assert(dbref);

    database_close(dbref);
    free(dbref);

    dbref = NULL;
}

int balance_get(uint64_t *sats, char *address)
{
    int r;
    size_t serialized_value_len = 0;
    unsigned char *serialized_value = NULL;

    r = database_get(&serialized_value, &serialized_value_len, dbref, (unsigned char *)address, strlen(address));
    ERROR_CHECK_NEG(r, "Could not get value from balance database.");

    if (!serialized_value)
    {
        return 0;
    }

    deserialize_uint64(sats, serialized_value, SERIALIZE_ENDIAN_BIG);

    free(serialized_value);

    return 1;
}

int balance_put(char *address, uint64_t sats)
{
    int r;
    unsigned char serialized[sizeof(uint64_t)];

    assert(address);

    serialize_uint64(serialized, sats, SERIALIZE_ENDIAN_BIG);

    r = database_put(dbref, (unsigned char *)address, strlen(address), serialized, sizeof(uint64_t));
    ERROR_CHECK_NEG(r, "Can not put new value in the balance database.");

    return 1;
}