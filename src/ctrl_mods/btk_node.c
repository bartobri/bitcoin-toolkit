#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "mods/node.h"
#include "mods/message.h"
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

	int o, i;
	char* host = NULL;
	int port = HOST_PORT;
	int message_type = MESSAGE_TYPE_VERSION;

	Node node;
	Message message;
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
		fprintf(stderr, "Please specify a target host with the -h command option.\n");
		return EXIT_FAILURE;
	}

	switch (message_type)
	{
		case MESSAGE_TYPE_VERSION:
			node = node_new(host, port);
			if (!node)
			{
				fprintf(stderr, "Could not connect to host %s on port %i\n", host, port);
				return EXIT_FAILURE;
			}

			// Construct and send a version message
			version_string_len = version_new_serialize(&version_string);
			message = message_new(VERSION_COMMAND, version_string, version_string_len);
			FREE(version_string);
			node_write_message(node, message);
			message_free(message);

			// Wait for version message response
			for (i = 0; i < TIMEOUT; ++i)
			{
				if ((message = node_get_message(node, VERSION_COMMAND)))
				{
					break;
				}
				sleep(1);
			}
			if (message == NULL)
			{
				fprintf(stderr, "Timeout Error. Did not receive version message response from target node.\n");
				return EXIT_FAILURE;
			}

			payload = NULL;
			payload_len = message_get_payload(&payload, message);

			version_deserialize(payload, &v, payload_len);
			json = version_to_json(v);

			printf("%s\n", json);

			message_free(message);
			FREE(payload);
			FREE(json);

			break;
	}

	return EXIT_SUCCESS;
}