/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "error.h"
#include "serialize.h"

int transaction_from_raw(Trans trans, unsigned char *input, size_t input_len)
{
	int r;
	size_t i;

	assert(trans);
	assert(input);
	assert(input_len);

	input = deserialize_uint32(&(trans->version), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&(trans->input_count), input, SERIALIZE_ENDIAN_LIT);

	trans->inputs = malloc(sizeof(TXInput) * trans->input_count);
	ERROR_CHECK_NULL(trans->inputs, "Memory allocation error");

	for (i = 0; i < trans->input_count; ++i)
	{
		(trans->inputs)[i] = malloc(sizeof(struct TXInput));
		ERROR_CHECK_NULL((trans->inputs)[i], "Memory allocation error.");

		r = txinput_from_raw((trans->inputs)[i], input);
		ERROR_CHECK_NEG(r, "Could not parse transaction inputs.");

		input += r;
	}

	input = deserialize_compuint(&(trans->output_count), input, SERIALIZE_ENDIAN_LIT);

	trans->outputs = malloc(sizeof(TXOutput) * trans->output_count);
	ERROR_CHECK_NULL(trans->outputs, "Memory allocation error");

	for (i = 0; i < trans->output_count; ++i)
	{
		(trans->outputs)[i] = malloc(sizeof(struct TXOutput));
		ERROR_CHECK_NULL((trans->outputs)[i], "Memory allocation error.");

		r = txoutput_from_raw((trans->outputs)[i], input);
		ERROR_CHECK_NEG(r, "Could not parse transaction inputs.");

		input += r;
	}

	input = deserialize_uint32(&(trans->lock_time), input, SERIALIZE_ENDIAN_LIT);

	return 1;
}

void trans_free(Trans trans)
{
	uint64_t i;

	assert(trans);
	assert(trans->inputs);
	assert(trans->outputs);

	for (i = 0; i < trans->input_count; i++)
	{
		assert(trans->inputs[i]);
		assert(trans->inputs[i]->script_raw);

		free(trans->inputs[i]->script_raw);
		free(trans->inputs[i]);
	}

	for (i = 0; i < trans->output_count; i++)
	{
		assert(trans->outputs[i]);
		assert(trans->outputs[i]->script_raw);

		free(trans->outputs[i]->script_raw);
		free(trans->outputs[i]);
	}

	free(trans->inputs);
	free(trans->outputs);

	free(trans);

	return;
}
