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

#define ADDRESSDB_PATH_SIZE                1000
#define ADDRESSDB_DEFAULT_PATH             ".bitcoin/address"

static DBRef dbref = -1;

int addressdb_open(char *p, bool create)
{
    int r;
    char path[ADDRESSDB_PATH_SIZE];

    memset(path, 0, ADDRESSDB_PATH_SIZE);

    if (p == NULL)
    {
        strcpy(path, getenv("HOME"));
        if (*path == 0)
        {
            error_log("Unable to determine home directory.");
            return -1;
        }
        strcat(path, "/");
        strcat(path, ADDRESSDB_DEFAULT_PATH);
    }
    else
    {
        strcpy(path, p);
    }

    r = database_open(&dbref, path, create);
    if (r < 0)
    {
        error_log("Error while opening UTXO database.");
        return -1;
    }

    return 1;
}

void addressdb_close(void)
{
    if (dbref > -1)
    {
        database_close(dbref);
    }
}

int addressdb_get(uint64_t *sats, char *address)
{
    int r;
    size_t serialized_value_len = 0;
    unsigned char *serialized_value = NULL;

    r = database_get(&serialized_value, &serialized_value_len, dbref, (unsigned char *)address, strlen(address));
    if (r < 0)
    {
        error_log("Could not get value from address database.");
        return -1;
    }

    if (serialized_value != NULL)
    {
        deserialize_uint64(sats, serialized_value, SERIALIZE_ENDIAN_BIG);
        free(serialized_value);
    }

    return 1;
}

int addressdb_put(char *address, uint64_t sats)
{
    int r;
    unsigned char serialized[sizeof(uint64_t)];

    assert(address);
    assert(sats > 0);

    serialize_uint64(serialized, sats, SERIALIZE_ENDIAN_BIG);

    r = database_put(dbref, (unsigned char *)address, strlen(address), serialized, sizeof(uint64_t));
    if (r < 0)
    {
        error_log("Can not put new value in the address database.");
        return -1;
    }

    return 1;
}