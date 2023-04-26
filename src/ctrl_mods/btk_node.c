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

int btk_node_main(output_item *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int r;
	unsigned char payload_send[BUFSIZ];
	unsigned char *payload_resp;
	size_t payload_send_len;
	size_t payload_resp_len;
	char *command_resp;
	char *json;
	Version version = NULL;
	Verack verack = NULL;

	assert(opts);

	(void)input;
	(void)input_len;

	if (strcmp(command, VERSION_COMMAND) == 0)
	{
		r = version_new(&version);
		ERROR_CHECK_NEG(r, "Could not create new version message.");

		r = version_to_raw(payload_send, version);
		ERROR_CHECK_NEG(r, "Could not serialize new version message.");

		payload_send_len = r;

		version_destroy(version);
	}
	else if (strcmp(command, VERACK_COMMAND) == 0)
	{
		r = verack_new(&verack);
		ERROR_CHECK_NEG(r, "Could not create new verack message.");

		r = verack_to_raw(payload_send, verack);
		ERROR_CHECK_NEG(r, "Could not serialize new version message.");

		payload_send_len = r;

		verack_destroy(verack);
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

	if (strcmp(command_resp, VERSION_COMMAND) == 0)
	{
		r = version_new_from_raw(&version, payload_resp, payload_resp_len);
		ERROR_CHECK_NEG(r, "Could not deserialize host response.");

		r = version_to_json(&json, version);
		ERROR_CHECK_NEG(r, "Could not convert version response to json.");

		version_destroy(version);
	}
	else if (strcmp(command_resp, VERACK_COMMAND) == 0)
	{
		// stub for now.
	}
	else
	{
		error_log("Unknown command response: %s.", command_resp);
		return -1;
	}

	*output = output_append_new_copy(*output, json, strlen(json));
	ERROR_CHECK_NULL(*output, "Memory allocation error.");

	free(json);
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

	r = node_read_message(&response, node);
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

	if (*payload_len > 0)
	{
		*payload = malloc(*payload_len);
		ERROR_CHECK_NULL(*payload, "Memory allocation error.");

		memcpy(*payload, message->payload, *payload_len);
	}
	else
	{
		*payload = NULL;
	}

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