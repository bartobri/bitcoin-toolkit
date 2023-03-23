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
#include "mods/json.h"
#include "mods/error.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x)  STRINGIFY2(x)

#define VERSION     70015
#define SERVICES    0x00
#define IP_ADDRESS  "00000000000000000000ffff7f000001"
#define PORT_MAIN   8333
#define PORT_TEST   18333
#define USER_AGENT  "/Bitcoin-Toolkit:" STRINGIFY(BTK_VERSION_MAJOR) "." STRINGIFY(BTK_VERSION_MINOR) "." STRINGIFY(BTK_VERSION_REVISION) "/"

int version_new(Version *version)
{
	int r;

	*version = malloc(sizeof(struct Version));
	ERROR_CHECK_NULL(*version, "Memory allocation error.");
	
	(*version)->version = VERSION;
	(*version)->services = SERVICES;
	(*version)->timestamp = time(NULL);
	(*version)->addr_recv_services = SERVICES;

	r = hex_str_to_raw((*version)->addr_recv_ip_address, IP_ADDRESS);
	ERROR_CHECK_NEG(r, "Could not convert hex string to raw data.");
	
	if (network_is_main())
	{
		(*version)->addr_recv_port = PORT_MAIN;
	}
	else
	{
		(*version)->addr_recv_port = PORT_TEST;
	}

	(*version)->addr_trans_services = SERVICES;
	
	r = hex_str_to_raw((*version)->addr_trans_ip_address, IP_ADDRESS);
	ERROR_CHECK_NEG(r, "Could not convert hex string to raw data.");
	
	if (network_is_main())
	{
		(*version)->addr_trans_port = PORT_MAIN;
	}
	else
	{
		(*version)->addr_trans_port = PORT_TEST;
	}

	(*version)->nonce = 0x00;
	(*version)->user_agent_bytes = (uint64_t)strlen(USER_AGENT);
	
	if ((*version)->user_agent_bytes)
	{
		(*version)->user_agent = malloc((*version)->user_agent_bytes + 1);
		ERROR_CHECK_NULL((*version)->user_agent, "Memory allocation error.");

		memset((*version)->user_agent, 0, (*version)->user_agent_bytes + 1);

		memcpy((*version)->user_agent, USER_AGENT, strlen(USER_AGENT));
	}

	(*version)->start_height = 0x00;
	(*version)->relay = 0x00;
	
	return 1;
}

int version_to_raw(unsigned char *output, Version version)
{
	unsigned char *head;

	assert(output);
	assert(version);

	head = output;
	
	output = serialize_uint32(output, version->version, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, version->services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, version->timestamp, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, version->addr_recv_services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uchar(output, version->addr_recv_ip_address, IP_ADDR_FIELD_LEN);
	output = serialize_uint16(output, version->addr_recv_port, SERIALIZE_ENDIAN_BIG);
	output = serialize_uint64(output, version->addr_trans_services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uchar(output, version->addr_trans_ip_address, IP_ADDR_FIELD_LEN);
	output = serialize_uint16(output, version->addr_trans_port, SERIALIZE_ENDIAN_BIG);
	output = serialize_uint64(output, version->nonce, SERIALIZE_ENDIAN_LIT);
	output = serialize_compuint(output, version->user_agent_bytes, SERIALIZE_ENDIAN_LIT);	
	output = serialize_char(output, version->user_agent, strlen(version->user_agent));
	output = serialize_uint32(output, version->start_height, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint8(output, version->relay, SERIALIZE_ENDIAN_LIT);

	return output - head;
}

int version_new_from_raw(Version *version, unsigned char *input, size_t input_len)
{
	unsigned char *head;

	assert(input);
	assert(input_len);

	head = input;

	*version = malloc(sizeof(struct Version));
	ERROR_CHECK_NULL(*version, "Memory allocation error.");
	
	input = deserialize_uint32(&((*version)->version), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&((*version)->services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&((*version)->timestamp), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&((*version)->addr_recv_services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar((*version)->addr_recv_ip_address, input, IP_ADDR_FIELD_LEN, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint16(&((*version)->addr_recv_port), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint64(&((*version)->addr_trans_services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar((*version)->addr_trans_ip_address, input, IP_ADDR_FIELD_LEN, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint16(&((*version)->addr_trans_port), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint64(&((*version)->nonce), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&((*version)->user_agent_bytes), input, SERIALIZE_ENDIAN_LIT);
	if ((*version)->user_agent_bytes > 0)
	{
		(*version)->user_agent = malloc((*version)->user_agent_bytes + 1);
		ERROR_CHECK_NULL((*version)->user_agent, "Memory allocation error.");

		memset((*version)->user_agent, 0, (*version)->user_agent_bytes + 1);

		input = deserialize_char((*version)->user_agent, input, (*version)->user_agent_bytes);
	}
	input = deserialize_uint32(&((*version)->start_height), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint8(&((*version)->relay), input, SERIALIZE_ENDIAN_LIT);

	return input - head;
}

int version_to_json(char **output, Version version)
{
	int i, r;
	cJSON *version_json = NULL;
	cJSON *tmp = NULL;
	char bitstr[100];
	char tmpstr[100];

	assert(version);

	r = json_init_object(&version_json);
	ERROR_CHECK_NEG(r, "Can not initialize json object.");

	r = json_add_number(version_json, version->version, "version");
	ERROR_CHECK_NEG(r, "Could not add version to json object.");

	{
		r = json_init_object(&tmp);
		ERROR_CHECK_NEG(r, "Can not initialize json object (tmp).");

		for (i = 0; i < 64; ++i)
		{
			if (((version->services >> i) & 0x0000000000000001) == 1)
			{
				sprintf(bitstr, "bit %d", i);

				r = json_add_string(tmp, version_service_bit_to_str(i), bitstr);
				ERROR_CHECK_NEG(r, "Could not add nonce to json object.");
			}
		}

		r = json_add_object(version_json, tmp, "services");
		ERROR_CHECK_NEG(r, "Could not add services to json object.");
	}

	r = json_add_number(version_json, version->timestamp, "timestamp");
	ERROR_CHECK_NEG(r, "Could not add timestamp to json object.");

	{
		r = json_init_object(&tmp);
		ERROR_CHECK_NEG(r, "Can not initialize json object (tmp).");

		for (i = 0; i < 64; ++i)
		{
			if (((version->addr_recv_services >> i) & 0x0000000000000001) == 1)
			{
				sprintf(bitstr, "bit %d", i);

				r = json_add_string(tmp, version_service_bit_to_str(i), bitstr);
				ERROR_CHECK_NEG(r, "Could not add nonce to json object.");
			}
		}

		r = json_add_object(version_json, tmp, "addr_recv_services");
		ERROR_CHECK_NEG(r, "Could not add addr_recv_services to json object.");
	}

	{
		for(i = 0; i < IP_ADDR_FIELD_LEN; ++i)
		{
			sprintf(tmpstr + (i*2), "%02x", version->addr_recv_ip_address[i]);
		}

		r = json_add_string(version_json, tmpstr, "addr_recv_ip_address");
		ERROR_CHECK_NEG(r, "Could not add addr_recv_ip_address to json object.");
	}

	r = json_add_number(version_json, version->addr_recv_port, "addr_recv_port");
	ERROR_CHECK_NEG(r, "Could not add addr_recv_port to json object.");

	{
		r = json_init_object(&tmp);
		ERROR_CHECK_NEG(r, "Can not initialize json object (tmp).");

		for (i = 0; i < 64; ++i)
		{
			if (((version->addr_trans_services >> i) & 0x0000000000000001) == 1)
			{
				sprintf(bitstr, "bit %d", i);

				r = json_add_string(tmp, version_service_bit_to_str(i), bitstr);
				ERROR_CHECK_NEG(r, "Could not add nonce to json object.");
			}
		}

		r = json_add_object(version_json, tmp, "addr_trans_services");
		ERROR_CHECK_NEG(r, "Could not add addr_trans_services to json object.");
	}

	{
		for(i = 0; i < IP_ADDR_FIELD_LEN; ++i)
		{
			sprintf(tmpstr + (i*2), "%02x", version->addr_trans_ip_address[i]);
		}

		r = json_add_string(version_json, tmpstr, "addr_trans_ip_address");
		ERROR_CHECK_NEG(r, "Could not add addr_trans_ip_address to json object.");
	}

	r = json_add_number(version_json, version->addr_trans_port, "addr_trans_port");
	ERROR_CHECK_NEG(r, "Could not add addr_trans_port to json object.");

	r = json_add_number(version_json, version->nonce, "nonce");
	ERROR_CHECK_NEG(r, "Could not add nonce to json object.");

	r = json_add_string(version_json, version->user_agent, "user_agent");
	ERROR_CHECK_NEG(r, "Could not add user_agent to json object.");

	r = json_add_number(version_json, version->start_height, "start_height");
	ERROR_CHECK_NEG(r, "Could not add start_height to json object.");

	r = json_add_bool(version_json, version->relay, "relay");
	ERROR_CHECK_NEG(r, "Could not add relay to json object.");

	r = json_to_string(output, version_json);
	ERROR_CHECK_NEG(r, "Could not convert json to string.");

	json_free(version_json);

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
		case 4:
			return "NODE_XTHIN";
			break;
		case 6:
			return "NODE_COMPACT_FILTERS";
			break;
		case 10:
			return "NODE_NETWORK_LIMITED";
			break;
	}

	return "UNKNOWN";
}

void version_destroy(Version version)
{
	assert(version);

	free(version->user_agent);
	free(version);
}