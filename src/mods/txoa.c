/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "txoa.h"
#include "error.h"
#include "database.h"
#include "serialize.h"
#include "transaction.h"

#define TXAO_LAST_BLOCK_KEY     "__last_block"
#define TXOA_KEY_LEN            TRANSACTION_ID_LEN + sizeof(uint32_t)
#define TXOA_DEFAULT_PATH       ".btk/balance/txoa"

static DBRef dbref = NULL;

int txoa_open(char *path, bool create)
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
		strcat(path, TXOA_DEFAULT_PATH);
	}

	r = database_open(&dbref, path, create);
	if (r < 0)
	{
		error_log("Could not open txoa database.");
		return -1;
	}

	return 1;
}

void txoa_close(void)
{
	assert (dbref);

	database_close(dbref);
	free(dbref);

	dbref = NULL;
}

int txoa_get(char *address, uint64_t *amount, unsigned char *tx_hash, uint32_t index)
{
	int r;
	size_t len;
	unsigned char key[TXOA_KEY_LEN];
	unsigned char *tmp;

	assert(address);
	assert(tx_hash);

	serialize_uchar(key, tx_hash, TRANSACTION_ID_LEN);
	serialize_uint32(key + TRANSACTION_ID_LEN, index, SERIALIZE_ENDIAN_LIT);

	r = database_get(&tmp, &len, dbref, key, TXOA_KEY_LEN);
	ERROR_CHECK_NEG(r, "Could not get address from txoa database.");

	if (len > sizeof(uint64_t))
	{
		memcpy(address, tmp, len - sizeof(uint64_t));
		deserialize_uint64(amount, tmp + (len - sizeof(uint64_t)), SERIALIZE_ENDIAN_LIT);
	}

	free(tmp);

	return 1;
}

int txoa_put(unsigned char *tx_hash, uint32_t index, char *address)
{
	int r;
	unsigned char key[TXOA_KEY_LEN];

	assert(tx_hash);
	assert(address);

	serialize_uchar(key, tx_hash, TRANSACTION_ID_LEN);
	serialize_uint32(key + TRANSACTION_ID_LEN, index, SERIALIZE_ENDIAN_LIT);

	r = database_put(dbref, key, TXOA_KEY_LEN, (unsigned char *)address, strlen(address));
	ERROR_CHECK_NEG(r, "Could not add entry to txoa database.");

	return 1;
}

int txoa_delete(unsigned char *tx_hash, uint32_t index)
{
	int r;
	unsigned char key[TXOA_KEY_LEN];

	assert(tx_hash);

	serialize_uchar(key, tx_hash, TRANSACTION_ID_LEN);
	serialize_uint32(key + TRANSACTION_ID_LEN, index, SERIALIZE_ENDIAN_LIT);

	r = database_delete(dbref, key, TXOA_KEY_LEN);
	ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");

	return 1;
}

int txoa_set_last_block(int block_num)
{
	int r;
	char *key;
	unsigned char value[sizeof(uint32_t)];

	key = TXAO_LAST_BLOCK_KEY;

	serialize_uint32(value, (uint32_t)block_num, SERIALIZE_ENDIAN_LIT);

	r = database_put(dbref, (unsigned char *)key, strlen(key), value, sizeof(uint32_t));
	ERROR_CHECK_NEG(r, "Could not add entry to txoa database.");

	return 1;
}

int txoa_get_last_block(int *block_num)
{
	int r;
	char *key;
	unsigned char *value = NULL;
	size_t value_len;

	key = TXAO_LAST_BLOCK_KEY;

	r = database_get(&value, &value_len, dbref, (unsigned char *)key, strlen(key));
	ERROR_CHECK_NEG(r, "Could not get address from txoa database.");

	if (!value)
	{
		return 0;
	}

	deserialize_uint32((uint32_t *)block_num, value, SERIALIZE_ENDIAN_LIT);

	free(value);

	return 1;
}

int txoa_batch_put(unsigned char *tx_hash, uint32_t index, char *address, uint64_t amount)
{
	int r;
	unsigned char key[TXOA_KEY_LEN];
	unsigned char value[BUFSIZ];

	assert(tx_hash);
	assert(address);

	memset(key, 0, TXOA_KEY_LEN);
	memset(value, 0, BUFSIZ);

	serialize_uchar(key, tx_hash, TRANSACTION_ID_LEN);
	serialize_uint32(key + TRANSACTION_ID_LEN, index, SERIALIZE_ENDIAN_LIT);

	memcpy(value, address, strlen(address));
	serialize_uint64(value + strlen(address), amount, SERIALIZE_ENDIAN_LIT);

	r = database_batch_put(dbref, key, TXOA_KEY_LEN, value, strlen(address) + sizeof(uint64_t));
	ERROR_CHECK_NEG(r, "Could not add entry to txoa database.");

	return 1;
}

int txoa_batch_delete(unsigned char *tx_hash, uint32_t index)
{
	int r;
	unsigned char key[TXOA_KEY_LEN];

	assert(tx_hash);

	serialize_uchar(key, tx_hash, TRANSACTION_ID_LEN);
	serialize_uint32(key + TRANSACTION_ID_LEN, index, SERIALIZE_ENDIAN_LIT);

	r = database_batch_delete(dbref, key, TXOA_KEY_LEN);
	ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");

	return 1;
}

int txoa_batch_write(void)
{
	int r;

	assert(dbref);

	r = database_batch_write(dbref);
	ERROR_CHECK_NEG(r, "Could not execute batch write.");

	return 1;
}

int txoa_get_record_count(size_t *count)
{
	int r;
	size_t c = 0;

	assert(count);
	assert(dbref);

	database_iter_reset(dbref);

	r = database_iter_seek_start(dbref);
	ERROR_CHECK_NEG(r, "Unable to seek to first record in txoa database.");

	while ((r = database_iter_next(dbref)) > 0)
	{
		c++;
	}
	ERROR_CHECK_NEG(r, "Could not iterate.");

	*count = c;

	return 1;
}