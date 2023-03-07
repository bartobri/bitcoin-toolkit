/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "crypto.h"
#include "network.h"
#include "message.h"
#include "serialize.h"
#include "error.h"

#define MESSAGE_MAINNET        0xD9B4BEF9
#define MESSAGE_TESTNET        0x0709110B

int message_new(Message m, const char *command, unsigned char *payload, size_t payload_len)
{
	int r;
	
	assert(m);
	assert(command);

	if (strlen(command) > MESSAGE_COMMAND_MAXLEN)
	{
		error_log("Command length (%i) can not exceed %i bytes in length.", (int)strlen(command), MESSAGE_COMMAND_MAXLEN);
		return -1;
	}

	if (network_is_main())
	{
		m->magic = MESSAGE_MAINNET;
	}
	else
	{
		m->magic = MESSAGE_TESTNET;
	}

	strncpy(m->command, command, MESSAGE_COMMAND_MAXLEN);
	m->length = payload_len;

	if (payload_len)
	{
		m->payload = malloc(payload_len);
		ERROR_CHECK_NULL(m->payload, "Memory allocation error.");

		memcpy(m->payload, payload, payload_len);

		r = crypto_get_checksum(&m->checksum, m->payload, (size_t)m->length);
		ERROR_CHECK_NEG(r, "Could not generate checksum for message payload.");
	}

	return 1;
}

int message_serialize(unsigned char *output, size_t *output_len, Message m)
{
	assert(output);
	assert(output_len);
	assert(m);

	output = serialize_uint32(output, m->magic, SERIALIZE_ENDIAN_LIT);
	output = serialize_char(output, m->command, MESSAGE_COMMAND_MAXLEN);
	output = serialize_uint32(output, m->length, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint32(output, m->checksum, SERIALIZE_ENDIAN_BIG);
	if (m->length)
	{
		output = serialize_uchar(output, m->payload, m->length);
	}

	*output_len = 12 + MESSAGE_COMMAND_MAXLEN + m->length;
	
	return 1;
}

int message_deserialize(Message output, unsigned char *input, size_t input_len)
{
	assert(output);
	assert(input);
	assert(input_len);

	if (input_len < 12 + MESSAGE_COMMAND_MAXLEN)
	{
		error_log("Input length (%i) insifficient to create a new message. At least %i bytes required.", input_len, 12 + MESSAGE_COMMAND_MAXLEN);
		return -1;
	}

	input = deserialize_uint32(&(output->magic), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_char(output->command, input, MESSAGE_COMMAND_MAXLEN);
	input = deserialize_uint32(&(output->length), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint32(&(output->checksum), input, SERIALIZE_ENDIAN_BIG);
	if (output->length)
	{
		output->payload = malloc(output->length);
		ERROR_CHECK_NULL(output->payload, "Memory allocation error.");

		input = deserialize_uchar(output->payload, input, output->length, SERIALIZE_ENDIAN_BIG);
	}
	
	return 12 + MESSAGE_COMMAND_MAXLEN + output->length;
}

int message_is_valid(Message m)
{
	int r;
	uint32_t checksum;

	assert(m);

	// If we don't have a payload then there is nothing to validate
	if (m->length == 0)
	{
		return 1;
	}

	r = crypto_get_checksum(&checksum, m->payload, (size_t)m->length);
	if (r < 0)
	{
		error_log("Could not generate checksum for message payload.");
		return -1;
	}

	return (checksum == m->checksum);
}

int message_is_complete(unsigned char *raw, size_t len)
{
	uint32_t payload_len;

	if (len < MESSAGE_MIN_SIZE)
	{
		return 0;
	}

	raw += sizeof(uint32_t);
	raw += MESSAGE_COMMAND_MAXLEN;

	deserialize_uint32(&payload_len, raw, SERIALIZE_ENDIAN_LIT);

	if (len < MESSAGE_MIN_SIZE + payload_len)
	{
		return 0;
	}

	return 1;
}