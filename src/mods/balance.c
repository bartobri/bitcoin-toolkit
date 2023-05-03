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
	int r, i;
	int len;
	char address_reverse[BUFSIZ];
	size_t serialized_value_len = 0;
	unsigned char *serialized_value = NULL;

	memset(address_reverse, 0, BUFSIZ);

	len = strlen(address);
	for (i = 0; i < len; i++)
	{
		address_reverse[i] = address[len - 1 - i];
	}

	r = database_get(&serialized_value, &serialized_value_len, dbref, (unsigned char *)address_reverse, strlen(address_reverse));
	ERROR_CHECK_NEG(r, "Could not get value from balance database.");

	if (!serialized_value)
	{
		return 0;
	}

	deserialize_uint64(sats, serialized_value, SERIALIZE_ENDIAN_BIG);

	free(serialized_value);

	return 1;
}

int balance_delete(char *address)
{
	int r, i;
	int len;
	char address_reverse[BUFSIZ];

	memset(address_reverse, 0, BUFSIZ);

	len = strlen(address);
	for (i = 0; i < len; i++)
	{
		address_reverse[i] = address[len - 1 - i];
	}

	r = database_delete(dbref, (unsigned char *)address_reverse, strlen(address_reverse));
	ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");

	return 1;
}

int balance_put(char *address, uint64_t sats)
{
	int r, i;
	int len;
	char address_reverse[BUFSIZ];
	unsigned char serialized[sizeof(uint64_t)];

	assert(address);

	memset(address_reverse, 0, BUFSIZ);

	len = strlen(address);
	for (i = 0; i < len; i++)
	{
		address_reverse[i] = address[len - 1 - i];
	}

	serialize_uint64(serialized, sats, SERIALIZE_ENDIAN_BIG);

	r = database_put(dbref, (unsigned char *)address_reverse, strlen(address_reverse), serialized, sizeof(uint64_t));
	ERROR_CHECK_NEG(r, "Can not put new value in the balance database.");

	return 1;
}

int balance_batch_put(char *address, uint64_t sats)
{
	int r, i;
	int len;
	char address_reverse[BUFSIZ];
	unsigned char serialized[sizeof(uint64_t)];

	assert(address);
	assert(dbref);

	memset(address_reverse, 0, BUFSIZ);

	len = strlen(address);
	for (i = 0; i < len; i++)
	{
		address_reverse[i] = address[len - 1 - i];
	}

	serialize_uint64(serialized, sats, SERIALIZE_ENDIAN_BIG);
	
	r = database_batch_put(dbref, (unsigned char *)address_reverse, strlen(address_reverse), serialized, sizeof(uint64_t));
	ERROR_CHECK_NEG(r, "Could not execute batch put.");

	return 1;
}

int balance_batch_delete(char *address)
{
	int r, i;
	int len;
	char address_reverse[BUFSIZ];

	memset(address_reverse, 0, BUFSIZ);

	len = strlen(address);
	for (i = 0; i < len; i++)
	{
		address_reverse[i] = address[len - 1 - i];
	}

	r = database_batch_delete(dbref, (unsigned char *)address_reverse, strlen(address_reverse));
	ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");

	return 1;
}

int balance_batch_write(void)
{
	int r;

	assert(dbref);

	r = database_batch_write(dbref);
	ERROR_CHECK_NEG(r, "Could not execute batch write.");

	return 1;
}

int balance_get_record_count(size_t *count)
{
	int r;
	size_t c = 0;

	assert(count);
	assert(dbref);

	database_iter_reset(dbref);

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