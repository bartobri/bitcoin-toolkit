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
	unsigned char *message_string;
	size_t message_string_len;


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

			// TODO - What happens if we use an invalid host?
			node = node_new(host, port);
			if (!node)
			{
				fprintf(stderr, "Could not connect to host %s on port %i\n", host, port);
				return EXIT_FAILURE;
			}

			// Construct and send a version message
			message_string_len = version_new_serialize(&message_string);
			message = message_new(VERSION_COMMAND, message_string, message_string_len);
			FREE(message_string);
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

			message_string_len = message_serialize(message, &message_string);
			for (i = 0; i < (int)message_string_len; ++i)
			{
				printf("%02x", message_string[i]);
			}
			printf("\n");
			message_free(message);

			// Wait for verack message response and send responding verack
			for (i = 0; i < TIMEOUT; ++i)
			{
				if ((message = node_get_message(node, VERACK_COMMAND)))
				{
					break;
				}
				sleep(1);
			}
			if (message)
			{
				// Construct and send a version message
				message_string_len = version_new_serialize(&message_string);
				message = message_new(VERACK_COMMAND, message_string, message_string_len);
				FREE(message_string);
				// TODO - set version number in message to lesser of the two
				//node_write_message(node, message);
				message_free(message);
			}

			break;
	}

	return EXIT_SUCCESS;
}