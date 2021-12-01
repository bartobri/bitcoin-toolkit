/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ctrl_mods/btk_help.h"
#include "ctrl_mods/btk_privkey.h"
#include "ctrl_mods/btk_pubkey.h"
#include "ctrl_mods/btk_vanity.h"
#include "ctrl_mods/btk_node.h"
#include "ctrl_mods/btk_utxodb.h"
#include "ctrl_mods/btk_addressdb.h"
#include "ctrl_mods/btk_version.h"
#include "mods/error.h"
#include "mods/input.h"

#define BTK_COMMAND_MAX_OPT 100

int btk_init(int argc, char *argv[]);
int btk_cleanup(char *);

int main(int argc, char *argv[])
{
	int i, r;
	char *command = NULL;
	char command_str[BUFSIZ];

	r = 0;

	// Assembling the original command string for logging purposes before we
	// pass the args to the control modules.
	memset(command_str, 0, BUFSIZ);
	for (i = 0; i < argc; ++i)
	{
		if (i > 0)
		{
			strncat(command_str, " ", BUFSIZ - 1 - strlen(command_str));
		}
		strncat(command_str, argv[i], BUFSIZ - 1 - strlen(command_str));
	}

	if (argc <= 1)
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Missing command parameter.");
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}

	// Save command for later referece.
	command = argv[1];

	// Turn off getopt errors for all control mods. I print my own 
	// error message.
	opterr = 0;

	// Run init function
	r = btk_init(argc, argv);
	if (r < 0)
	{
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}

	// This loop provides support for list processing
	do
	{
		if (strcmp(command, "help") == 0)
		{
			r = btk_help_main();
		}
		else if (strcmp(command, "privkey") == 0)
		{
			r = btk_privkey_main();
		}
		else if (strcmp(command, "pubkey") == 0)
		{
			r = btk_pubkey_main();
		}
		else if (strcmp(command, "vanity") == 0)
		{
			r = btk_vanity_main();
		}
		else if (strcmp(command, "node") == 0)
		{
			r = btk_node_main();
		}
		else if (strcmp(command, "utxodb") == 0)
		{
			r = btk_utxodb_main();
		}
		else if (strcmp(command, "addressdb") == 0)
		{
			r = btk_addressdb_main();
		}
		else if (strcmp(command, "version") == 0)
		{
			r = btk_version_main();
		}
		else
		{
			error_log("See 'btk help' to read about available commands.");
			error_log("'%s' is not a valid command.", command);
			error_log("Error [%s]:", command_str);
			error_print();
			return EXIT_FAILURE;
		}

		if (r < 0)
		{
			error_log("Error [%s]:", command_str);
			error_print();
			return EXIT_FAILURE;
		}
	}
	while (input_available());

	// Run cleanup function
	r = btk_cleanup(command);
	if (r < 0)
	{
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int btk_init(int argc, char *argv[])
{
	int r = 0;
	char *command = NULL;

	command = argv[1];

	if (strcmp(command, "help") == 0)
	{
		r = btk_help_init(argc, argv);
	}
	else if (strcmp(command, "privkey") == 0)
	{
		r = btk_privkey_init(argc, argv);
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		r = btk_pubkey_init(argc, argv);
	}
	else if (strcmp(command, "vanity") == 0)
	{
		r = btk_vanity_init(argc, argv);
	}
	else if (strcmp(command, "node") == 0)
	{
		r = btk_node_init(argc, argv);
	}
	else if (strcmp(command, "utxodb") == 0)
	{
		r = btk_utxodb_init(argc, argv);
	}
	else if (strcmp(command, "addressdb") == 0)
	{
		r = btk_addressdb_init(argc, argv);
	}
	else if (strcmp(command, "version") == 0)
	{
		r = btk_version_init(argc, argv);
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Invalid command: '%s'", command);
		return -1;
	}

	if (r < 0)
	{
		error_log("Can not run initialization for command '%s'.", command);
		return -1;
	}

	return 1;
}

int btk_cleanup(char *command)
{
	int r = 0;

	if (strcmp(command, "help") == 0)
	{
		r = btk_help_cleanup();
	}
	else if (strcmp(command, "privkey") == 0)
	{
		r = btk_privkey_cleanup();
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		r = btk_pubkey_cleanup();
	}
	else if (strcmp(command, "vanity") == 0)
	{
		r = btk_vanity_cleanup();
	}
	else if (strcmp(command, "node") == 0)
	{
		r = btk_node_cleanup();
	}
	else if (strcmp(command, "utxodb") == 0)
	{
		r = btk_utxodb_cleanup();
	}
	else if (strcmp(command, "addressdb") == 0)
	{
		r = btk_addressdb_cleanup();
	}
	else if (strcmp(command, "version") == 0)
	{
		r = btk_version_cleanup();
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Invalid command: '%s'", command);
		return -1;
	}

	if (r < 0)
	{
		error_log("Can not run cleanup for command %s.", command);
		return -1;
	}

	return 1;
}
