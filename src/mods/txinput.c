/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "txinput.h"
#include "hex.h"
#include "error.h"
#include "serialize.h"

int txinput_from_raw(TXInput txinput, unsigned char *input, size_t input_len)
{
	unsigned char *head;

	assert(txinput);
	assert(input);
	assert(input_len);

	head = input;

	input = deserialize_uchar(txinput->tx_hash, input, TXINPUT_TSHASH_LENGTH);
	input = deserialize_uint32(&(txinput->index), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&(txinput->script_size), input, SERIALIZE_ENDIAN_LIT);

	(txinput->script_raw) = malloc(txinput->script_size);
	ERROR_CHECK_NULL((txinput->script_raw), "Memory allocation error.");

	input = deserialize_uchar(txinput->script_raw, input, txinput->script_size);
	input = deserialize_uint32(&(txinput->sequence), input, SERIALIZE_ENDIAN_LIT);

	return (input - head);
}
