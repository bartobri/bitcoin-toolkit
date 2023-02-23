/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "txoutput.h"
#include "hex.h"
#include "error.h"
#include "serialize.h"

int txoutput_from_raw(TXOutput txoutput, unsigned char *input)
{
	unsigned char *head;
	
	assert(txoutput);
	assert(input);

	head = input;

	txoutput->amount = 0;
	txoutput->script_size = 0;
	
	input = deserialize_uint64(&(txoutput->amount), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&(txoutput->script_size), input, SERIALIZE_ENDIAN_LIT);

	(txoutput->script_raw) = malloc(txoutput->script_size);
	ERROR_CHECK_NULL((txoutput->script_raw), "Memory allocation error.");

	input = deserialize_uchar(txoutput->script_raw, input, txoutput->script_size, SERIALIZE_ENDIAN_BIG);

	return (input - head);
}
