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

int message_new(Message *message, const char *command, unsigned char *payload, size_t payload_len)
{
	int r;
	
	assert(command);
	assert(strlen(command) <= MESSAGE_COMMAND_MAXLEN);

	*message = malloc(sizeof(struct Message));
	ERROR_CHECK_NULL(*message, "Memory allocation error.");

	if (network_is_main())
	{
		(*message)->magic = MESSAGE_MAINNET;
	}
	else
	{
		(*message)->magic = MESSAGE_TESTNET;
	}

	strncpy((*message)->command, command, MESSAGE_COMMAND_MAXLEN);
	(*message)->length = payload_len;

	if ((*message)->length)
	{
		(*message)->payload = malloc((*message)->length);
		ERROR_CHECK_NULL((*message)->payload, "Memory allocation error.");

		memcpy((*message)->payload, payload, (*message)->length);
	}

	r = crypto_get_checksum(&((*message)->checksum), (*message)->payload, (size_t)(*message)->length);
	ERROR_CHECK_NEG(r, "Could not generate checksum for message payload.");

	return 1;
}

int message_to_raw(unsigned char *output, Message message)
{
	unsigned char *head;

	assert(output);
	assert(message);

	head = output;

	output = serialize_uint32(output, message->magic, SERIALIZE_ENDIAN_LIT);
	output = serialize_char(output, message->command, MESSAGE_COMMAND_MAXLEN);
	output = serialize_uint32(output, message->length, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint32(output, message->checksum, SERIALIZE_ENDIAN_BIG);
	if (message->length)
	{
		output = serialize_uchar(output, message->payload, message->length);
	}
	
	return output - head;
}

int message_new_from_raw(Message *message, unsigned char *input)
{
	unsigned char *head;

	assert(input);

	*message = malloc(sizeof(struct Message));
	ERROR_CHECK_NULL(*message, "Memory allocation error.");

	head = input;

	input = deserialize_uint32(&((*message)->magic), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_char((*message)->command, input, MESSAGE_COMMAND_MAXLEN);
	input = deserialize_uint32(&((*message)->length), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint32(&((*message)->checksum), input, SERIALIZE_ENDIAN_BIG);
	if ((*message)->length)
	{
		(*message)->payload = malloc((*message)->length);
		ERROR_CHECK_NULL((*message)->payload, "Memory allocation error.");

		input = deserialize_uchar((*message)->payload, input, (*message)->length, SERIALIZE_ENDIAN_BIG);
	}

	return input - head;
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

int message_get_payload_len(uint32_t *len, unsigned char *raw)
{
	raw += sizeof(uint32_t);
	raw += MESSAGE_COMMAND_MAXLEN;

	deserialize_uint32(len, raw, SERIALIZE_ENDIAN_LIT);

	return 1;
}

void message_destroy(Message message)
{
	assert(message);

	if (message->length > 0)
	{
		free(message->payload);
	}
	free(message);
}