/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "error.h"
#include "database.h"
#include "utxokey.h"
#include "utxovalue.h"

#define CHAINSTATE_DEFAULT_PATH             ".bitcoin/chainstate"
#define CHAINSTATE_OFUSCATE_KEY_KEY         "\016\000obfuscate_key"
#define CHAINSTATE_OFUSCATE_KEY_KEY_LENGTH  15

static DBRef dbref = NULL;
static unsigned char *obfuscate_key = NULL;
static size_t obfuscate_key_len = 0;

int chainstate_open(char *path)
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
		strcat(path, CHAINSTATE_DEFAULT_PATH);
	}

	r = database_open(&dbref, path, false);
	ERROR_CHECK_NEG(r, "Could not open chainstate database.");

	r = database_get(&obfuscate_key, &obfuscate_key_len, dbref, (unsigned char *)CHAINSTATE_OFUSCATE_KEY_KEY, CHAINSTATE_OFUSCATE_KEY_KEY_LENGTH);
	ERROR_CHECK_NEG(r, "Could not get obfuscate key from chainstate database.");
	ERROR_CHECK_NULL(obfuscate_key, "No obfuscate key returned from chainstate database");

	// Drop first char
	obfuscate_key++;
	obfuscate_key_len -= 1;

	return 1;
}

int chainstate_seek_start(void)
{
	int r;

	assert(dbref);

	r = database_iter_seek_start(dbref);
	ERROR_CHECK_NEG(r, "Unable to seek to first record in chainstate database.");

	// Skip past first two records because they have specific uses.
	database_iter_next(dbref);
	database_iter_next(dbref);

	return 1;
}

int chainstate_get_next(UTXOKey key, UTXOValue value)
{
	int r;
	size_t i;
	size_t serialized_key_len = 0;
	size_t serialized_value_len = 0;
	unsigned char *serialized_key = NULL;
	unsigned char *serialized_value = NULL;

	assert(key);
	assert(value);

	r = database_iter_get(&serialized_key, &serialized_key_len, &serialized_value, &serialized_value_len, dbref);
	ERROR_CHECK_NEG(r, "Could not get data from database.");

	// De-obfuscate the value
	for (i = 0; i < serialized_value_len; i++)
	{
		serialized_value[i] ^= obfuscate_key[i % obfuscate_key_len];
	}

	r = utxokey_from_raw(key, serialized_key);
	ERROR_CHECK_NEG(r, "Could not deserialize key data.");

	r = utxovalue_from_raw(value, serialized_value);
	ERROR_CHECK_NEG(r, "Could not deserialize value data.");

	free(serialized_key);
	free(serialized_value);

	r = database_iter_next(dbref);
	ERROR_CHECK_NEG(r, "Could not set chainstate iter to next record.")
	if (r == 0)
	{
		// End of database
		return 0;
	}

	return 1;
}

int chainstate_get_record_count(size_t *count)
{
	int r;
	size_t c = 0;

	assert(dbref);

	r = database_iter_seek_start(dbref);
	ERROR_CHECK_NEG(r, "Unable to seek to first record in chainstate database.");

	while ((r = database_iter_next(dbref)) > 0)
	{
		c++;
	}
	ERROR_CHECK_NEG(r, "Could not iterate.");

	*count = c;

	return 1;
}

void chainstate_close(void)
{
	assert(dbref);

	database_close(dbref);
	free(dbref);

	dbref = NULL;
}