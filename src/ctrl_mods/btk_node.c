/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "mods/network.h"
#include "mods/node.h"
#include "mods/message.h"
#include "mods/output.h"
#include "mods/opts.h"
#include "mods/json.h"
#include "mods/error.h"
#include "mods/commands/version.h"
#include "mods/commands/verack.h"
#include "mods/cJSON/cJSON.h"

#define HOST_PORT_MAIN       "8333"
#define HOST_PORT_TEST       "18333"

static Node node;
static int message_command;

int btk_node_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int i, r;
	cJSON *version_json = NULL;
	cJSON *tmp = NULL;
	char bitstr[100];
	char tmpstr[100];

	unsigned char *node_data = NULL;

	Message message;
	unsigned char *message_raw;
	size_t message_raw_len;

	Version version = NULL;
	unsigned char *version_string;
	size_t version_string_len;
	char *json = NULL;

	assert(opts);

	(void)input;
	(void)input_len;

	if (message_command == MESSAGE_COMMAND_VERSION)
	{
		version_string = malloc(version_sizeof());
		ERROR_CHECK_NULL(version_string, "Memory allocation error.");

		r = version_new_serialize(version_string);
		ERROR_CHECK_NEG(r, "Could not serialize version data.");

		version_string_len = r;

		message = malloc(sizeof(struct Message));
		ERROR_CHECK_NULL(message, "Memory allocation error.");

		r = message_new(message, VERSION_COMMAND, version_string, version_string_len);
		ERROR_CHECK_NEG(r, "Could not create a new message.");

		free(version_string);

		message_raw = malloc(sizeof(struct Message) + message->length);
		ERROR_CHECK_NULL(message_raw, "Memory allocation error.");

		r = message_to_raw(message_raw, message);
		ERROR_CHECK_NEG(r, "Could not serialize message data.");

		message_raw_len = r;

		r = node_write(node, message_raw, message_raw_len);
		ERROR_CHECK_NEG(r, "Could not send message to host.");

		free(message);
		free(message_raw);

		r = node_read(node, &node_data, NODE_READ_BREAK_MESSAGE);
		ERROR_CHECK_NEG(r, "Could not read message from host.");

		message = malloc(sizeof(struct Message));
		ERROR_CHECK_NULL(message, "Memory allocation error.");

		r = message_from_raw(message, node_data);
		ERROR_CHECK_NEG(r, "Could not deserialize message from host.");

		r = message_is_valid(message);
		ERROR_CHECK_NEG(r, "Could not validate message from host.");
		ERROR_CHECK_FALSE(r, "The message received from host contains an invalid checksum.");
		
		if (strncmp(message->command, VERSION_COMMAND, MESSAGE_COMMAND_MAXLEN) != 0)
		{
			r = json_init(&tmp);
			ERROR_CHECK_NEG(r, "Can not initialize json object (tmp).");

			r = json_add_string(tmp, opts->host_name, "host");
			ERROR_CHECK_NEG(r, "Could not add nonce to json object.");

			r = json_add_string(tmp, "Unexpect message returned.", "output");
			ERROR_CHECK_NEG(r, "Could not add nonce to json object.");

			r = json_to_string(&json, tmp);
			ERROR_CHECK_NEG(r, "Could not convert json to string.");

			*output = output_append_new_copy(*output, json, strlen(json));
			ERROR_CHECK_NULL(*output, "Memory allocation error.");

			json_free(tmp);

			free(json);

			return 1;
		}
		
		version = malloc(version_sizeof());
		ERROR_CHECK_NULL(version, "Memory allocation error.");

		r = version_deserialize(version, message->payload, message->length);
		ERROR_CHECK_NEG(r, "Could not deserialize host response.");

		r = json_init(&version_json);
		ERROR_CHECK_NEG(r, "Can not initialize json object.");

		r = json_add_string(version_json, opts->host_name, "host");
		ERROR_CHECK_NEG(r, "Could not add nonce to json object.");

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

		r = json_to_string(&json, version_json);
		ERROR_CHECK_NEG(r, "Could not convert json to string.");

		*output = output_append_new_copy(*output, json, strlen(json));
		ERROR_CHECK_NULL(*output, "Memory allocation error.");

		json_free(version_json);

		free(json);
		free(version);
		free(message);
		free(node_data);
	}

	return 1;
}

int btk_node_requires_input(opts_p opts)
{
    assert(opts);

    return 0;
}

int btk_node_init(opts_p opts)
{
	int r;

	ERROR_CHECK_NULL(opts->host_name, "Missing hostname argument.");

	// Force list output format so it prints the json in output obj.
	opts->output_format_list = 1;

	// Set default service if needed
	if (!opts->host_service)
	{
		if (opts->network_test)
		{
			opts->host_service = HOST_PORT_TEST;
		}
		else
		{
			opts->host_service = HOST_PORT_MAIN;
		}
	}

	// Connect to node
	r = node_new(&node, opts->host_name, opts->host_service);
	ERROR_CHECK_NEG(r, "Could not get new node object.");

	r = node_connect(node);
	ERROR_CHECK_NEG(r, "Could not connect to host.");

	// Only supporting version command for now.
	message_command = MESSAGE_COMMAND_VERSION;

	return 1;
}

int btk_node_cleanup(opts_p opts)
{
	(void)opts;

	node_disconnect(node);
	node_destroy(node);
	
	return 1;
}