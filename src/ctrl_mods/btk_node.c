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
#include "mods/opts.h"
#include "mods/error.h"
#include "mods/commands/version.h"
#include "mods/commands/verack.h"

// Defaults
static char *host_name   = OPTS_HOST_NAME_NONE;
static int host_port     = OPTS_HOST_PORT_NONE;
static int network       = OPTS_OUTPUT_NETWORK_MAINNET;

#define HOST_PORT_MAIN       8333
#define HOST_PORT_TEST       18333
#define TIMEOUT              10
#define MESSAGE_TYPE_VERSION 1

int btk_node_main(opts_p opts)
{
	int i, r;
	int message_type = MESSAGE_TYPE_VERSION;

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

	// Override defaults
	if (opts->host_name) { host_name = opts->host_name; }
	if (opts->host_port) { host_port = opts->host_port; }
	if (opts->network) { network = opts->network; }

	ERROR_CHECK_NULL(host_name, "Missing host argument.");

	if (!host_port)
	{
		if (network == OPTS_OUTPUT_NETWORK_MAINNET)
		{
			host_port = HOST_PORT_MAIN;
		}
		else if (network == OPTS_OUTPUT_NETWORK_TESTNET)
		{
			host_port = HOST_PORT_TEST;
		}
	}
	
	switch (message_type)
	{
		case MESSAGE_TYPE_VERSION:
			node = malloc(node_sizeof());
			ERROR_CHECK_NULL(node, "Memory allocation error.");

			r = node_connect(node, host_name, host_port);
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