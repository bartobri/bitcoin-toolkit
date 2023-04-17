/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "utxokey.h"
#include "serialize.h"

int utxokey_from_raw(UTXOKey key, unsigned char *input)
{
	unsigned char *head;

	assert(key);
	assert(input);

	head = input;

	input = deserialize_uint8(&(key->type), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uchar(key->tx_hash, input, UTXOKEY_TX_HASH_LENGTH, SERIALIZE_ENDIAN_LIT);
	input = deserialize_varint(&(key->vout), input);

	return (input - head);
}

void utxokey_free(UTXOKey key)
{
	free(key);
}