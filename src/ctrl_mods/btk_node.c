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
#include "mods/network.h"
#include "mods/node.h"
#include "mods/message.h"
#include "mods/error.h"
#include "mods/commands/version.h"
#include "mods/commands/verack.h"

#define HOST_PORT_MAIN       8333
#define HOST_PORT_TEST       18333
#define TIMEOUT              10
#define MESSAGE_TYPE_VERSION 1

static char* host = NULL;
static int port   = HOST_PORT_MAIN;

int btk_node_init(int argc, char *argv[])
{
	int o;

	while ((o = getopt(argc, argv, "h:p:T")) != -1)
	{
		switch (o)
		{
			case 'h':
				host = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'T':
				port = HOST_PORT_TEST;
				network_set_test();
				break;
			case '?':
				error_log("See 'btk help %s' to read about available argument options.", argv[1]);
				if (isprint(optopt))
				{
					error_log("Invalid command option '-%c'.", optopt);
				}
				else
				{
					error_log("Invalid command option character '\\x%x'.", optopt);
				}
				return -1;
		}
	}

	if (host == NULL)
	{
		error_log("See 'btk help %s' to read about available argument options.", argv[1]);
		error_log("Missing host argument.");
		return -1;
	}

	return 1;
}

int btk_node_main(void)
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

	switch (message_type)
	{
		case MESSAGE_TYPE_VERSION:
			node = malloc(node_sizeof());
			if (node == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			r = node_connect(node, host, port);
			if (r < 0)
			{
				error_log("Could not connect to host.");
				return -1;
			}

			version_string = malloc(version_sizeof());
			if (version_string == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			r = version_new_serialize(version_string);
			if (r < 0)
			{
				error_log("Could not serialize version data.");
				return -1;
			}
			version_string_len = r;

			message = malloc(message_sizeof());
			if (message == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			r = message_new(message, VERSION_COMMAND, version_string, version_string_len);
			if (r < 0)
			{
				error_log("Could not create a new message.");
				return -1;
			}
			free(version_string);

			message_raw = malloc(message_sizeof());
			if (message_raw == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			r = message_serialize(message_raw, &message_raw_len, message);
			if (r < 0)
			{
				error_log("Could not serialize message data.");
				return -1;
			}

			r = node_write(node, message_raw, message_raw_len);
			if (r < 0)
			{
				error_log("Could not send message to host.");
				return -1;
			}
			free(message);
			free(message_raw);

			for (i = 0; i < TIMEOUT; ++i)
			{
				node_data = NULL;
				node_data_len = 0;
				r = node_read(node, &node_data);
				if (r < 0)
				{
					error_log("Could not read message from host.");
					return -1;
				}

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
				if (message == NULL)
				{
					error_log("Memory allocation error.");
					return -1;
				}

				r = message_deserialize(message, node_data_walk, node_data_len);
				if (r < 0)
				{
					error_log("Could not deserialize message from host.");
					return -1;
				}
				node_data_len -= r;
				node_data_walk += r;

				r = message_is_valid(message);
				if (r < 0)
				{
					error_log("Could not validate message from host.");
					return -1;
				}
				if (r == 0)
				{
					error_log("The message received from host contains an invalid checksum.");
					return -1;
				}

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
			
			if (message == NULL)
			{
				printf("Did not receive response from host before timeout.\n");
				return 1;
			}
			
			payload = malloc(message_get_payload_len(message));
			if (payload == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			payload_len = message_get_payload(payload, message);
			
			version = malloc(version_sizeof());
			if (version == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			r = version_deserialize(version, payload, payload_len);
			if (r < 0)
			{
				error_log("Could not deserialize host response.");
				return -1;
			}

			json = malloc(1000);
			if (json == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

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

int btk_node_cleanup(void)
{
	return 1;
}