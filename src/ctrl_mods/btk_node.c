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
#include "mods/network.h"
#include "mods/node.h"
#include "mods/message.h"
#include "mods/output.h"
#include "mods/opts.h"
#include "mods/error.h"
#include "mods/commands/version.h"
#include "mods/commands/verack.h"

#define HOST_PORT_MAIN       8333
#define HOST_PORT_TEST       18333
#define TIMEOUT              10
#define MESSAGE_TYPE_VERSION 1

int btk_node_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int i, r;
	int message_type = MESSAGE_TYPE_VERSION;

	(void)output;

	Node node;
	unsigned char *node_data, *node_data_walk;
	size_t node_data_len;

	Message message;
	unsigned char *message_raw;
	size_t message_raw_len;
	unsigned char *payload = NULL;
	size_t payload_len;

	Version version = NULL;
	unsigned char *version_string;
	size_t version_string_len;
	char *json = NULL;

	assert(opts);

	(void)input;
	(void)input_len;

	ERROR_CHECK_NULL(opts->host_name, "Missing host argument.");

	if (!opts->host_port)
	{
		if (opts->network_test)
		{
			opts->host_port = HOST_PORT_TEST;
		}
		else
		{
			opts->host_port = HOST_PORT_MAIN;
		}
	}
	
	switch (message_type)
	{
		case MESSAGE_TYPE_VERSION:
			node = malloc(node_sizeof());
			ERROR_CHECK_NULL(node, "Memory allocation error.");

			r = node_connect(node, opts->host_name, opts->host_port);
			ERROR_CHECK_NEG(r, "Could not connect to host.");

			version_string = malloc(version_sizeof());
			ERROR_CHECK_NULL(version_string, "Memory allocation error.");

			r = version_new_serialize(version_string);
			ERROR_CHECK_NEG(r, "Could not serialize version data.");

			version_string_len = r;

			message = malloc(message_sizeof());
			ERROR_CHECK_NULL(message, "Memory allocation error.");

			r = message_new(message, VERSION_COMMAND, version_string, version_string_len);
			ERROR_CHECK_NEG(r, "Could not create a new message.");

			free(version_string);

			message_raw = malloc(message_sizeof());
			ERROR_CHECK_NULL(message_raw, "Memory allocation error.");

			r = message_serialize(message_raw, &message_raw_len, message);
			ERROR_CHECK_NEG(r, "Could not serialize message data.");

			r = node_write(node, message_raw, message_raw_len);
			ERROR_CHECK_NEG(r, "Could not send message to host.");

			free(message);
			free(message_raw);

			for (i = 0; i < TIMEOUT; ++i)
			{
				node_data = NULL;
				node_data_len = 0;

				r = node_read(node, &node_data);
				ERROR_CHECK_NEG(r, "Could not read message from host.");

				if (r > 0)
				{
					node_data_len = r;
					break;
				}

				sleep(1);
			}

			node_disconnect(node);

			node_data_walk = node_data;
			message = NULL;

			while (node_data_len > 0)
			{
				message = malloc(message_sizeof());
				ERROR_CHECK_NULL(message, "Memory allocation error.");

				r = message_deserialize(message, node_data_walk, node_data_len);
				ERROR_CHECK_NEG(r, "Could not deserialize message from host.");

				node_data_len -= r;
				node_data_walk += r;

				r = message_is_valid(message);
				ERROR_CHECK_NEG(r, "Could not validate message from host.");
				ERROR_CHECK_FALSE(r, "The message received from host contains an invalid checksum.");

				r = message_cmp_command(message, VERSION_COMMAND);
				if (r == 0)
				{
					break;
				}
				else
				{
					free(message);
					message = NULL;
				}
			}
			
			ERROR_CHECK_NULL(message, "Did not receive response from host before timeout.");
			
			payload = malloc(message_get_payload_len(message));
			ERROR_CHECK_NULL(payload, "Memory allocation error.");

			payload_len = message_get_payload(payload, message);
			
			version = malloc(version_sizeof());
			ERROR_CHECK_NULL(version, "Memory allocation error.");

			r = version_deserialize(version, payload, payload_len);
			ERROR_CHECK_NEG(r, "Could not deserialize host response.");

			json = malloc(1000);
			ERROR_CHECK_NULL(json, "Memory allocation error.");

			version_to_json(json, version);

			printf("%s\n", json);

			free(version);
			free(message);
			free(payload);
			free(json);
			free(node);
			free(node_data);

			break;
	}

	return 1;
}

int btk_node_requires_input(opts_p opts)
{
    assert(opts);

    return 0;
}