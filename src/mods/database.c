/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <leveldb/c.h>
#include "database.h"
#include "error.h"

static leveldb_t *db;

int database_open(char *location)
{
    char *err = NULL;
    leveldb_options_t *options;

    options = leveldb_options_create();
    db = leveldb_open(options, location, &err);

    if (err != NULL) {
        error_log("Unable to open the database: %s.", err);
        return -1;
    }

    return 1;
}

int database_is_open(void)
{
    if (db == NULL)
    {
        return 0;
    }

    return 1;
}

int database_get(unsigned char **output, size_t *output_len, char *key, size_t key_len)
{
    char *err = NULL;
    leveldb_readoptions_t *roptions;

    *output_len = 0;

    if (database_is_open())
    {
        roptions = leveldb_readoptions_create();
        *output = (unsigned char *)leveldb_get(db, roptions, key, key_len, output_len, &err);

        if (err != NULL) {
            error_log("Unable to get key value: %s.", err);
            return -1;
        }

        if (*output_len == 0)
        {
            error_log("Key does not exist.");
            return -1;
        }
    }
    else
    {
        error_log("Unable to get key value. Database has not been opened.");
        return -1;
    }

    return 1;
}

void database_close(void)
{
    if (database_is_open())
    {
        leveldb_close(db);
    }
}