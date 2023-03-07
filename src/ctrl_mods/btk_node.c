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
static char *command;

int btk_node_send(char *, unsigned char *, size_t);
int btk_node_response(char **, unsigned char **, size_t *);

int btk_node_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int i, r;
	cJSON *version_json = NULL;
	cJSON *tmp = NULL;
	char bitstr[100];
	char tmpstr[100];

	unsigned char *node_data = NULL;

	//Message message;
	Version version = NULL;

	unsigned char *payload_send;
	unsigned char *payload_resp;
	size_t payload_send_len;
	size_t payload_resp_len;

	char *json = NULL;

	char *command_resp;

	assert(opts);

	(void)input;
	(void)input_len;

	if (strcmp(command, VERSION_COMMAND) == 0)
	{
		payload_send = malloc(version_sizeof());
		ERROR_CHECK_NULL(payload_send, "Memory allocation error.");

		r = version_new_serialize(payload_send);
		ERROR_CHECK_NEG(r, "Could not serialize version data.");

		payload_send_len = r;
	}
	else
	{
		error_log("Unknown command string: %s.", command);
		return -1;
	}

	r = btk_node_send(command, payload_send, payload_send_len);
	ERROR_CHECK_NEG(r, "Could not send data.");

	r = btk_node_response(&command_resp, &payload_resp, &payload_resp_len);
	ERROR_CHECK_NEG(r, "Could not get node response.");



	
	if (strncmp(command_resp, VERSION_COMMAND, MESSAGE_COMMAND_MAXLEN) != 0)
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

	r = version_deserialize(version, payload_resp, payload_resp_len);
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
	free(node_data);
	free(payload_send);
	free(payload_resp);
	free(command_resp);

	return 1;
}

int btk_node_send(char *command, unsigned char *payload, size_t payload_len)
{
	int r;
	unsigned char message_raw[BUFSIZ];
	size_t message_raw_len;
	Message message;

	r = message_new(&message, command, payload, payload_len);
	ERROR_CHECK_NEG(r, "Could not create a new message.");

	r = message_to_raw(message_raw, message);
	ERROR_CHECK_NEG(r, "Could not serialize message data.");

	message_raw_len = r;

	r = node_write(node, message_raw, message_raw_len);
	ERROR_CHECK_NEG(r, "Could not send message to host.");

	message_destroy(message);

	return 1;
}

int btk_node_response(char **command, unsigned char **payload, size_t *payload_len)
{
	int r;
	unsigned char *response;
	Message message;

	r = node_read(node, &response, NODE_READ_BREAK_MESSAGE);
	ERROR_CHECK_NEG(r, "Could not read message from host.");

	r = message_is_complete(response, (size_t)r);
	ERROR_CHECK_FALSE(r, "Received incomplete message.");

	r = message_new_from_raw(&message, response);
	ERROR_CHECK_NEG(r, "Could not deserialize message from host.");

	r = message_is_valid(message);
	ERROR_CHECK_NEG(r, "Could not validate message from host.");
	ERROR_CHECK_FALSE(r, "The message received from host contains an invalid checksum.");

	*command = malloc(MESSAGE_COMMAND_MAXLEN);
	ERROR_CHECK_NULL(*command, "Memory allocation error.");

	strncpy(*command, message->command, MESSAGE_COMMAND_MAXLEN);

	*payload_len = message->length;

	*payload = malloc(*payload_len);
	ERROR_CHECK_NULL(*payload, "Memory allocation error.");

	memcpy(*payload, message->payload, *payload_len);

	message_destroy(message);
	free(response);

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
	command = VERSION_COMMAND;

	return 1;
}

int btk_node_cleanup(opts_p opts)
{
	(void)opts;

	node_disconnect(node);
	node_destroy(node);
	
	return 1;
}