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
		memcpy((*version)->user_agent, USER_AGENT, strlen(USER_AGENT));
	}

	(*version)->start_height = 0x00;
	(*version)->relay = 0x00;
	
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
		memset(output->user_agent, 0, USER_AGENT_MAX_LEN);
		input = deserialize_char(output->user_agent, input, output->user_agent_bytes);
	}
	input = deserialize_uint32(&(output->start_height), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint8(&(output->relay), input, SERIALIZE_ENDIAN_LIT);
	
	return 1;
}

size_t version_sizeof(void)
{
	return sizeof(struct Version);
}

int version_to_json(char **output, Version version)
{
	int i, r;
	cJSON *version_json = NULL;
	cJSON *tmp = NULL;
	char bitstr[100];
	char tmpstr[100];

	assert(version);

	r = json_init(&version_json);
	ERROR_CHECK_NEG(r, "Can not initialize json object.");

	r = json_add_number(version_json, version->version, "version");
	ERROR_CHECK_NEG(r, "Could not add version to json object.");

	{
		r = json_init(&tmp);
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
		r = json_init(&tmp);
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
		r = json_init(&tmp);
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

	free(version);
}