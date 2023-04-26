/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "utxovalue.h"
#include "serialize.h"
#include "camount.h"
#include "error.h"

#define UTXOVALUE_HASH160_PUBKEY_SIZE     20
#define UTXOVALUE_COMPRESSED_PUBKEY_SIZE  33

int utxovalue_from_raw(UTXOValue value, unsigned char *input)
{
	unsigned char *head;

	assert(value);
	assert(input);

	head = input;

	input = deserialize_varint(&(value->height), input);
	input = deserialize_varint(&(value->amount), input);
	input = deserialize_varint(&(value->n_size), input);
	if (value->n_size == 0 || value->n_size == 1)
	{
		value->script_len = UTXOVALUE_HASH160_PUBKEY_SIZE;
	}
	else if (value->n_size == 2 || value->n_size == 3)
	{
		// nsize byte is part of the script
		input--;

		value->script_len = UTXOVALUE_COMPRESSED_PUBKEY_SIZE;
	}
	else if (value->n_size == 4 || value->n_size == 5)
	{
		// nsize byte (-2) is is part of the script
		input--;
		*input -= 2;

		value->script_len = UTXOVALUE_COMPRESSED_PUBKEY_SIZE;
	}
	else
	{
		value->script_len = value->n_size - 6;
	}

	value->script = malloc(value->script_len);
	ERROR_CHECK_NULL(value->script, "Memory allocation error.");

	input = deserialize_uchar(value->script, input, value->script_len, SERIALIZE_ENDIAN_BIG);

	// pop off the coinbase flag from height.
	value->height = value->height >> 1;

	// Uncompress amount
	camount_uncompress(&(value->amount), value->amount);

	return (input - head);
}

void utxovalue_free(UTXOValue value)
{
	assert(value);

	if (value->script)
	{
		free(value->script);
	}
	free(value);
}