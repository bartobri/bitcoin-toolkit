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

struct DBRef {
    leveldb_t *db;
    leveldb_iterator_t *db_iter;
};

int database_open(DBRef *ref, char *location, bool create)
{
    char *err = NULL;
    leveldb_options_t *options;
    leveldb_readoptions_t *roptions;

#ifdef _NO_LEVELDB
    error_log("To fix this, install the leveldb development library, then recompile and reinstall this program.");
    error_log("The leveldb library was not detected at compile time. This feature has been disabled.");
    return -1;
#endif

    *ref = malloc(sizeof(struct DBRef));
    ERROR_CHECK_NULL(*ref, "Memory allocation error.");

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

    (*ref)->db = leveldb_open(options, location, &err);

    if (err != NULL) {
        error_log("Error: %s.", err);
        return -1;
    }

    roptions = leveldb_readoptions_create();
    (*ref)->db_iter = leveldb_create_iterator((*ref)->db, roptions);

    return 1;
}

int database_iter_seek_start(DBRef ref)
{
    assert(ref);

    leveldb_iter_seek_to_first(ref->db_iter);

    return 1;
}

int database_iter_seek_key(DBRef ref, unsigned char *key, size_t key_len)
{
    assert(ref);

    leveldb_iter_seek(ref->db_iter, (char *)key, key_len);

    if (!leveldb_iter_valid(ref->db_iter))
    {
        // End of database. Return zero.
        return 0;
    }

    return 1;
}

int database_iter_next(DBRef ref)
{
    assert(ref);
    assert(leveldb_iter_valid(ref->db_iter));

    leveldb_iter_next(ref->db_iter);

    if (!leveldb_iter_valid(ref->db_iter))
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

    assert(ref);
    assert(leveldb_iter_valid(ref->db_iter));

    tmp_key = leveldb_iter_key(ref->db_iter, key_len);

    *key = malloc(*key_len);
    ERROR_CHECK_NULL(*key, "Memory allocation error.");

    memcpy(*key, tmp_key, *key_len);

    tmp_value = leveldb_iter_value(ref->db_iter, value_len);

    *value = malloc(*value_len);
    ERROR_CHECK_NULL(*key, "Memory allocation error.");

    memcpy(*value, tmp_value, *value_len);

    return 1;
}

int database_iter_get_value(unsigned char **value, size_t *value_len, DBRef ref)
{
    const char *output;

    assert(ref);
    assert(leveldb_iter_valid(ref->db_iter));

    output = leveldb_iter_value(ref->db_iter, value_len);

    *value = malloc(*value_len);
    ERROR_CHECK_NULL(*value, "Memory allocation error.");

    memcpy(*value, output, *value_len);

    return 1;
}

int database_get(unsigned char **output, size_t *output_len, DBRef ref, unsigned char *key, size_t key_len)
{
    char *err = NULL;
    leveldb_readoptions_t *roptions;

    assert(ref);
    assert(key);

    roptions = leveldb_readoptions_create();

    *output_len = 0;
    *output = (unsigned char *)leveldb_get(ref->db, roptions, (char *)key, key_len, output_len, &err);

    if (err != NULL) {
        error_log("The database reported the following error: %s.", err);
        return -1;
    }

    leveldb_readoptions_destroy(roptions);

    return 1;
}

int database_put(DBRef ref, unsigned char *key, size_t key_len, unsigned char *value, size_t value_len)
{
    char *err = NULL;
    leveldb_writeoptions_t *woptions;

    assert(ref);
    assert(key);
    assert(value);
    
    woptions = leveldb_writeoptions_create();
    leveldb_put(ref->db, woptions, (char *)key, key_len, (char *)value, value_len, &err);

    if (err != NULL) {
        error_log("The database reported the following error: %s.", err);
        return -1;
    }

    leveldb_writeoptions_destroy(woptions);

    return 1;
}

int database_delete(DBRef ref, unsigned char *key, size_t key_len)
{
    char *err = NULL;
    leveldb_writeoptions_t *woptions;

    assert(ref);
    assert(key);

    woptions = leveldb_writeoptions_create();
    leveldb_delete(ref->db, woptions, (char *)key, key_len, &err);

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
    assert(ref);

    leveldb_iter_destroy(ref->db_iter);
    leveldb_close(ref->db);
}