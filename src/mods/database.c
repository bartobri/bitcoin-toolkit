/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <leveldb/c.h>
#include "database.h"
#include "error.h"

static leveldb_t *db;
static leveldb_iterator_t *iter;

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

int database_iter_seek_start(void)
{
    leveldb_readoptions_t *roptions;

    if (!database_is_open())
    {
        error_log("Database is not open.");
        return -1;
    }

    if (iter == NULL)
    {
        roptions = leveldb_readoptions_create();
        iter = leveldb_create_iterator(db, roptions);
    }

    leveldb_iter_seek_to_first(iter);

    return 1;
}

int database_iter_seek_key(unsigned char *key, size_t key_len)
{
    leveldb_readoptions_t *roptions;

    if (!database_is_open())
    {
        error_log("Database is not open.");
        return -1;
    }

    if (iter == NULL)
    {
        roptions = leveldb_readoptions_create();
        iter = leveldb_create_iterator(db, roptions);
    }

    leveldb_iter_seek(iter, (char *)key, key_len);

    if (!leveldb_iter_valid(iter))
    {
        // End of database. Return zero.
        return 0;
    }

    return 1;
}

int database_iter_next(void)
{
    if (!leveldb_iter_valid(iter))
    {
        error_log("Invalid database iterator.");
        return -1;
    }

    leveldb_iter_next(iter);

    if (!leveldb_iter_valid(iter))
    {
        // Reached end of database. Return zero.
        return 0;
    }

    return 1;
}

int database_iter_get(unsigned char **key, size_t *key_len, unsigned char **value, size_t *value_len)
{
    const char *output;

    if (iter == NULL)
    {
        error_log("Must seek database iterator before getting next value.");
        return -1;
    }

    if (!leveldb_iter_valid(iter))
    {
        error_log("Invalid database iterator.");
        return -1;
    }

    output = leveldb_iter_key(iter, key_len);
    *key = malloc(*key_len);
    if (*key == NULL)
    {
        error_log("Memory allocation error.");
        return -1;
    }
    memcpy(*key, output, *key_len);

    output = leveldb_iter_value(iter, value_len);
    *value = malloc(*value_len);
    if (*value == NULL)
    {
        error_log("Memory allocation error.");
        return -1;
    }
    memcpy(*value, output, *value_len);

    return 1;
}

int database_iter_get_value(unsigned char **value, size_t *value_len)
{
    const char *output;

    if (iter == NULL)
    {
        error_log("Must seek database iterator before getting next value.");
        return -1;
    }

    if (!leveldb_iter_valid(iter))
    {
        error_log("Invalid database iterator.");
        return -1;
    }

    output = leveldb_iter_value(iter, value_len);
    *value = malloc(*value_len);
    if (*value == NULL)
    {
        error_log("Memory allocation error.");
        return -1;
    }
    memcpy(*value, output, *value_len);

    return 1;
}

int database_get(unsigned char **output, size_t *output_len, unsigned char *key, size_t key_len)
{
    char *err = NULL;
    leveldb_readoptions_t *roptions;

    *output_len = 0;

    if (database_is_open())
    {
        roptions = leveldb_readoptions_create();
        *output = (unsigned char *)leveldb_get(db, roptions, (char *)key, key_len, output_len, &err);

        if (err != NULL) {
            error_log("The database reported the following error: %s.", err);
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