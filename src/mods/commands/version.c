/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>
#include "version.h"
#include "mods/config.h"
#include "mods/network.h"
#include "mods/serialize.h"
#include "mods/hex.h"
#include "mods/error.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x)  STRINGIFY2(x)

#define VERSION     70015
#define SERVICES    0x00
#define IP_ADDRESS  "00000000000000000000ffff7f000001"
#define PORT_MAIN   8333
#define PORT_TEST   18333
#define USER_AGENT  "/Bitcoin-Toolkit:" STRINGIFY(BTK_VERSION_MAJOR) "." STRINGIFY(BTK_VERSION_MINOR) "." STRINGIFY(BTK_VERSION_REVISION) "/"

int version_new(Version v)
{
	int r;

	assert(v);
	
	v->version = VERSION;
	v->services = SERVICES;
	v->timestamp = time(NULL);
	v->addr_recv_services = SERVICES;

	if (strlen(IP_ADDRESS) / 2 > IP_ADDR_FIELD_LEN)
	{
		error_log("Invalid IP address.");
		return -1;
	}

	r = hex_str_to_raw(v->addr_recv_ip_address, IP_ADDRESS);
	if (r < 0)
	{
		error_log("Could not convert hex string to raw data.");
		return -1;
	}
	
	if (network_is_main())
	{
		v->addr_recv_port = PORT_MAIN;
	}
	else
	{
		v->addr_recv_port = PORT_TEST;
	}

	v->addr_trans_services = SERVICES;
	
	r = hex_str_to_raw(v->addr_trans_ip_address, IP_ADDRESS);
	if (r < 0)
	{
		error_log("Could not convert hex string to raw data.");
		return -1;
	}
	
	if (network_is_main())
	{
		v->addr_trans_port = PORT_MAIN;
	}
	else
	{
		v->addr_trans_port = PORT_TEST;
	}

	v->nonce = 0x00;
	v->user_agent_bytes = (uint64_t)strlen(USER_AGENT);
	
	if (v->user_agent_bytes)
	{
		memcpy(v->user_agent, USER_AGENT, strlen(USER_AGENT));
	}

	v->start_height = 0x00;
	v->relay = 0x00;
	
	return 1;
}

int version_serialize(unsigned char *output, Version v)
{
	int r;
	unsigned char *head;

	assert(output);
	assert(v);

	r = 0;
	head = output;
	
	output = serialize_uint32(output, v->version, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, v->services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, v->timestamp, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, v->addr_recv_services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uchar(output, v->addr_recv_ip_address, IP_ADDR_FIELD_LEN);
	output = serialize_uint16(output, v->addr_recv_port, SERIALIZE_ENDIAN_BIG);
	output = serialize_uint64(output, v->addr_trans_services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uchar(output, v->addr_trans_ip_address, IP_ADDR_FIELD_LEN);
	output = serialize_uint16(output, v->addr_trans_port, SERIALIZE_ENDIAN_BIG);
	output = serialize_uint64(output, v->nonce, SERIALIZE_ENDIAN_LIT);
	output = serialize_compuint(output, v->user_agent_bytes, SERIALIZE_ENDIAN_LIT);	
	output = serialize_char(output, v->user_agent, strlen(v->user_agent));
	output = serialize_uint32(output, v->start_height, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint8(output, v->relay, SERIALIZE_ENDIAN_LIT);
	
	while (head++ != output)
	{
		r++;
	}

	return r;
}

int version_new_serialize(unsigned char *output)
{
	int r;
	Version v;

	assert(output);

	v = malloc(sizeof(*v));
	if (v == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = version_new(v);
	if (r < 0)
	{
		error_log("Could not generate new version message.");
		return -1;
	}
	
	r = version_serialize(output, v);
	if (r < 0)
	{
		error_log("Could not serialize new version message.");
		return -1;
	}
	
	free(v);
	
	return r;
}

int version_deserialize(Version output, unsigned char *input, size_t input_len)
{
	assert(output);
	assert(input);
	assert(input_len);

	if (input_len < 85)
	{
		error_log("Length of input is insufficient to deserialize all data needed for a version command.");
		return -1;
	}
	
	input = deserialize_uint32(&(output->version), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&(output->services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&(output->timestamp), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&(output->addr_recv_services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar(output->addr_recv_ip_address, input, IP_ADDR_FIELD_LEN, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint16(&(output->addr_recv_port), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint64(&(output->addr_trans_services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar(output->addr_trans_ip_address, input, IP_ADDR_FIELD_LEN, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint16(&(output->addr_trans_port), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint64(&(output->nonce), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&(output->user_agent_bytes), input, SERIALIZE_ENDIAN_LIT);
	if (output->user_agent_bytes)
	{
		if (input_len < 85 + output->user_agent_bytes)
		{
			error_log("Length of input is too short to accommodate the user agent field size.");
			return -1;
		}
		input = deserialize_char(output->user_agent, input, output->user_agent_bytes);
	}
	input = deserialize_uint32(&(output->start_height), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint8(&(output->relay), input, SERIALIZE_ENDIAN_LIT);
	
	return 1;
}

char *version_service_bit_to_str(int bit)
{
	assert(bit >= 0 && bit < 64);

	switch (bit)
	{
		case 0:
			return "NODE_NETWORK";
			break;
		case 1:
			return "NODE_GETUTXO";
			break;
		case 2:
			return "NODE_BLOOM";
			break;
		case 3:
			return "NODE_WITNESS";
			break;
		case 10:
			return "NODE_NETWORK_LIMITED";
			break;
	}

	return "UNKNOWN";
}

size_t version_sizeof(void)
{
	return sizeof(struct Version);
}