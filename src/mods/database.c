/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "database.h"
#include "error.h"
#ifdef _NO_LEVELDB
#include "leveldb/stub.h"
#else
#include <leveldb/c.h>
#endif

#define DATABASE_MAX_DB_OBJS 2

static leveldb_t *(db[DATABASE_MAX_DB_OBJS]);
static leveldb_iterator_t *(iter[DATABASE_MAX_DB_OBJS]);
static leveldb_readoptions_t *roptions;

int database_open(DBRef *ref, char *location, bool create)
{
    int i;
    char *err = NULL;
    leveldb_options_t *options;

#ifdef _NO_LEVELDB
    error_log("To fix this, install the leveldb development library, then recompile and reinstall this program.");
    error_log("The leveldb library was not detected at compile time. This feature has been disabled.");
    return -1;
#endif

    // Next available database reference slot
    for (i = 0; i < DATABASE_MAX_DB_OBJS && database_is_open(i); i++)
        ;

    options = leveldb_options_create();

    // Turn off snappy compression. If left on, reading from the db with this
    // will corrupt the database for bitcoin core.
    leveldb_options_set_compression(options, leveldb_no_compression);

    // Set create if missing option
    leveldb_options_set_create_if_missing(options, create);

    // If create is true, error if exists.
    if (create == true)
    {
        leveldb_options_set_error_if_exists(options, true);
    }

    db[i] = leveldb_open(options, location, &err);

    if (err != NULL) {
        error_log("Error: %s.", err);
        return -1;
    }

    roptions = leveldb_readoptions_create();
    iter[i] = leveldb_create_iterator(db[i], roptions);

    *ref = i;

    return 1;
}

int database_is_open(DBRef ref)
{
    if (db[ref] == NULL)
    {
        return 0;
    }

    return 1;
}

int database_iter_seek_start(DBRef ref)
{
    if (!database_is_open(ref))
    {
        error_log("Database is not open.");
        return -1;
    }

    leveldb_iter_seek_to_first(iter[ref]);

    return 1;
}

int database_iter_seek_key(DBRef ref, unsigned char *key, size_t key_len)
{
    if (!database_is_open(ref))
    {
        error_log("Database is not open.");
        return -1;
    }

    leveldb_iter_seek(iter[ref], (char *)key, key_len);

    if (!leveldb_iter_valid(iter[ref]))
    {
        // End of database. Return zero.
        return 0;
    }

    return 1;
}

int database_iter_next(DBRef ref)
{
    if (!leveldb_iter_valid(iter[ref]))
    {
        error_log("Invalid database iterator.");
        return -1;
    }

    leveldb_iter_next(iter[ref]);

    if (!leveldb_iter_valid(iter[ref]))
    {
        // Reached end of database. Return zero.
        return 0;
    }

    return 1;
}

int database_iter_get(unsigned char **key, size_t *key_len, unsigned char **value, size_t *value_len, DBRef ref)
{
    const char *tmp_key;
    const char *tmp_value;

    assert(iter[ref]);
    assert(leveldb_iter_valid(iter[ref]));

    tmp_key = leveldb_iter_key(iter[ref], key_len);

    *key = malloc(*key_len);
    ERROR_CHECK_NULL(*key, "Memory allocation error.");

    memcpy(*key, tmp_key, *key_len);

    tmp_value = leveldb_iter_value(iter[ref], value_len);

    *value = malloc(*value_len);
    ERROR_CHECK_NULL(*key, "Memory allocation error.");

    memcpy(*value, tmp_value, *value_len);

    return 1;
}

int database_iter_get_value(unsigned char **value, size_t *value_len, DBRef ref)
{
    const char *output;

    assert(iter[ref]);
    assert(leveldb_iter_valid(iter[ref]));

    output = leveldb_iter_value(iter[ref], value_len);

    *value = malloc(*value_len);
    ERROR_CHECK_NULL(*value, "Memory allocation error.");

    memcpy(*value, output, *value_len);

    return 1;
}

int database_get(unsigned char **output, size_t *output_len, DBRef ref, unsigned char *key, size_t key_len)
{
    char *err = NULL;

    assert(db[ref]);
    assert(database_is_open(ref));

    *output_len = 0;

    if (database_is_open(ref))
    {
        *output = (unsigned char *)leveldb_get(db[ref], roptions, (char *)key, key_len, output_len, &err);

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

int database_put(DBRef ref, unsigned char *key, size_t key_len, unsigned char *value, size_t value_len)
{
    char *err = NULL;
    leveldb_writeoptions_t *woptions;

    if (database_is_open(ref))
    {
        woptions = leveldb_writeoptions_create();
        leveldb_put(db[ref], woptions, (char *)key, key_len, (char *)value, value_len, &err);

        if (err != NULL) {
            error_log("The database reported the following error: %s.", err);
            return -1;
        }

        leveldb_writeoptions_destroy(woptions);
    }
    else
    {
        error_log("Unable to put key/value. Database has not been opened.");
        return -1;
    }

    return 1;
}

int database_delete(DBRef ref, unsigned char *key, size_t key_len)
{
    char *err = NULL;
    leveldb_writeoptions_t *woptions;

    assert(db[ref] != NULL);

    woptions = leveldb_writeoptions_create();

    leveldb_delete(db[ref], woptions, (char *)key, key_len, &err);

    if (err != NULL)
    {
        error_log("The database reported the following error: %s.", err);
        return -1;
    }

    leveldb_writeoptions_destroy(woptions);

    return 1;
}

void database_close(DBRef ref)
{
    if (database_is_open(ref))
    {
        if (iter[ref])
        {
            leveldb_iter_destroy(iter[ref]);
        }
        else
        {
            leveldb_readoptions_destroy(roptions);
        }
        leveldb_close(db[ref]);

        db[ref] = NULL;
        iter[ref] = NULL;
    }
}