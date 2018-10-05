#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "mods/node.h"
#include "mods/message.h"
#include "mods/error.h"
#include "mods/mem.h"
#include "mods/commands/version.h"
#include "mods/commands/verack.h"

#define HOST_PORT            8333
#define TIMEOUT              10
#define MESSAGE_TYPE_VERSION 1

int btk_node_main(int argc, char *argv[], unsigned char* input, size_t input_len)
{
	// Temporarily suppressing warnings for these
	(void)input;
	(void)input_len;

	int o, i, r;
	char* host = NULL;
	int port = HOST_PORT;
	int message_type = MESSAGE_TYPE_VERSION;

	Node node;
	unsigned char *node_data, *node_data_walk;
	size_t node_data_len;

	Message message;
	unsigned char *message_raw;
	size_t message_raw_len;
	unsigned char *payload = NULL;
	size_t payload_len;

	Version v = NULL;
	unsigned char *version_string;
	size_t version_string_len;
	char *json = NULL;


	while ((o = getopt(argc, argv, "h:p:")) != -1)
	{
		switch (o)
		{
			// Input format
			case 'h':
				host = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;

			// Unknown option
			case '?':
				if (isprint(optopt))
				{
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				}
				else
				{
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				}
				return EXIT_FAILURE;
		}
	}

	if (host == NULL)
	{
		error_log("Missing argument. Specify a hostname with the -h option.");
		error_print();
		return EXIT_FAILURE;
	}

	switch (message_type)
	{
		case MESSAGE_TYPE_VERSION:
			node = ALLOC(node_sizeof());
			r = node_connect(node, host, port);
			if (r < 0)
			{
				error_log("Error while connecting to host.");
				error_print();
				return EXIT_FAILURE;
			}

			version_string = ALLOC(version_sizeof());
			r = version_new_serialize(version_string);
			if (r < 0)
			{
				error_log("Error while serializing version data.");
				error_print();
				return EXIT_FAILURE;
			}
			version_string_len = r;

			message = ALLOC(message_sizeof());
			r = message_new(message, VERSION_COMMAND, version_string, version_string_len);
			if (r < 0)
			{
				error_log("Error while creating message.");
				error_print();
				return EXIT_FAILURE;
			}
			FREE(version_string);

			message_raw = ALLOC(message_sizeof());
			r = message_serialize(message_raw, &message_raw_len, message);
			if (r < 0)
			{
				error_log("Error while serializing message message.");
				error_print();
				return EXIT_FAILURE;
			}

			r = node_write(node, message_raw, message_raw_len);
			if (r < 0)
			{
				error_log("Error while sending message to host.");
				error_print();
				return EXIT_FAILURE;
			}
			FREE(message);
			FREE(message_raw);

			for (i = 0; i < TIMEOUT; ++i)
			{
				node_data = NULL;
				node_data_len = 0;
				r = node_read(node, &node_data);
				if (r < 0)
				{
					error_log("Error while reading message from host.");
					error_print();
					return EXIT_FAILURE;
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

			while (node_data_len > 0)
			{
				message = ALLOC(message_sizeof());
				r = message_deserialize(message, node_data_walk, node_data_len);
				if (r < 0)
				{
					error_log("Error while deserializing message from host.");
					error_print();
					return EXIT_FAILURE;
				}
				node_data_len -= r;
				node_data_walk += r;

				r = message_is_valid(message);
				if (r < 0)
				{
					error_log("Error while validating message from host.");
					error_print();
					return EXIT_FAILURE;
				}
				if (r == 0)
				{
					error_log("The message received from host contains an invalid checksum.");
					error_print();
					return EXIT_FAILURE;
				}

				r = message_cmp_command(message, VERSION_COMMAND);
				if (r == 0)
				{
					break;
				}
				else
				{
					FREE(message);
					message = NULL;
				}
			}
			
			if (message == NULL)
			{
				error_log("Timeout Error. Did not receive response from host.");
				error_print();
				return EXIT_FAILURE;
			}
			
			payload = ALLOC(message_get_payload_len(message));
			payload_len = message_get_payload(payload, message);
			
			v = ALLOC(version_sizeof());
			r = version_deserialize(v, payload, payload_len);
			if (r < 0)
			{
				error_log("Error while deserializing host response.");
				error_print();
				return EXIT_FAILURE;
			}

			json = ALLOC(1000);
			version_to_json(json, v);

			printf("%s\n", json);

			FREE(v);
			FREE(message);
			FREE(payload);
			FREE(json);
			FREE(node);
			FREE(node_data);

			break;
	}

	return EXIT_SUCCESS;
}