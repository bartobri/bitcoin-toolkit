/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "block.h"
#include "transaction.h"
#include "error.h"
#include "serialize.h"

int block_from_raw(Block block, unsigned char *input)
{
	int r;
	size_t i;
	unsigned char *head;

	assert(block);
	assert(input);

	head = input;

	input = deserialize_uint32(&(block->version), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar(block->prev_block, input, BLOCK_PREV_LEN, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar(block->merkel_root, input, BLOCK_MERKEL_ROOT_LEN, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint32(&(block->timestamp), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint32(&(block->bits), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint32(&(block->nonce), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&(block->tx_count), input, SERIALIZE_ENDIAN_LIT);

	block->transactions = malloc(sizeof(Trans) * block->tx_count);
	ERROR_CHECK_NULL(block->transactions, "Memory allocation error");

	for (i = 0; i < block->tx_count; i++)
	{
		(block->transactions)[i] = malloc(sizeof(struct Trans));
		ERROR_CHECK_NULL((block->transactions)[i], "Memory allocation error.");

		r = transaction_from_raw((block->transactions)[i], input);
		ERROR_CHECK_NEG(r, "Could not parse transaction inputs.");

		input += r;
	}

	return (input - head);
}

void block_free(Block block)
{
	uint64_t i;

	assert(block);

	for (i = 0; i < block->tx_count; i++)
	{
		transaction_free((block->transactions)[i]);
	}

	free(block->transactions);
	free(block);
}