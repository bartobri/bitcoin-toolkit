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

int main(int argc, char *argv[])
{
	int i, r;
	char command_str[BUFSIZ];
	char *argv_copy[BTK_COMMAND_MAX_OPT];

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

	// Turn off getopt errors for all control mods. I print my own 
	// error message.
	opterr = 0;

	do
	{
		optind = 0;

		// Get a fresh copy of argv for each pass through the loop. 
		// This is necessary for list processing because getopt()
		// alters it when processing.
		for (i = 0; i < argc; i++)
		{
			argv_copy[i] = malloc(strlen(argv[i]) + 1);
			ERROR_CHECK_NULL(argv_copy[i], "Unable to allocate memory.");
			strcpy(argv_copy[i], argv[i]);
		}

		// Execute the apropriate command module
		if (strcmp(argv_copy[1], "help") == 0)
		{
			r = btk_help_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "privkey") == 0)
		{
			r = btk_privkey_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "pubkey") == 0)
		{
			r = btk_pubkey_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "vanity") == 0)
		{
			r = btk_vanity_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "node") == 0)
		{
			r = btk_node_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "utxodb") == 0)
		{
			r = btk_utxodb_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "addressdb") == 0)
		{
			r = btk_addressdb_main(argc, argv_copy);
		}
		else if (strcmp(argv_copy[1], "version") == 0)
		{
			r = btk_version_main(argc, argv_copy);
		}
		else
		{
			error_log("See 'btk help' to read about available commands.");
			error_log("'%s' is not a valid command.", argv_copy[1]);
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

		// Free the argv copy so it can be used again on next pass.
		for (i = 0; i < argc; i++)
		{
			free(argv_copy[i]);
			argv_copy[i] = NULL;
		}
	}
	while (input_available());

	return EXIT_SUCCESS;
}
