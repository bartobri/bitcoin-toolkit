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
#include "mods/opts.h"
#include "mods/error.h"
#include "ctrl_mods/btk_help.h"
#include "ctrl_mods/btk_privkey.h"
#include "ctrl_mods/btk_pubkey.h"
#include "ctrl_mods/btk_address.h"
#include "ctrl_mods/btk_node.h"
#include "ctrl_mods/btk_utxodb.h"
#include "ctrl_mods/btk_addressdb.h"
#include "ctrl_mods/btk_version.h"

#define BTK_CHECK_NEG(x)            if (x < 0) { error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }

int main(int argc, char *argv[])
{
	int i, r = 0;
	char *command = NULL;
	char command_str[BUFSIZ];
	opts_p opts;

	// Assembling the original command string for logging purposes
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
	else
	{
		command = argv[1];
	}

	if (strcmp(command, "privkey") == 0)
	{
		opts = malloc(sizeof(*opts));
		ERROR_CHECK_FALSE(opts, "Memory allocation error.");

		r = opts_get(opts, argc, argv);
		BTK_CHECK_NEG(r);

		r = btk_privkey_main(opts);
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		opts = malloc(sizeof(*opts));
		ERROR_CHECK_FALSE(opts, "Memory allocation error.");
		
		r = opts_get(opts, argc, argv);
		BTK_CHECK_NEG(r);

		r = btk_pubkey_main(opts);
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "address") == 0)
	{
		opts = malloc(sizeof(*opts));
		ERROR_CHECK_FALSE(opts, "Memory allocation error.");
		
		r = opts_get(opts, argc, argv);
		BTK_CHECK_NEG(r);

		r = btk_address_main(opts);
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "node") == 0)
	{
		opts = malloc(sizeof(*opts));
		ERROR_CHECK_FALSE(opts, "Memory allocation error.");
		
		r = opts_get(opts, argc, argv);
		BTK_CHECK_NEG(r);

		r = btk_node_main(opts);
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "utxodb") == 0)
	{
		opts = malloc(sizeof(*opts));
		ERROR_CHECK_FALSE(opts, "Memory allocation error.");
		
		r = opts_get(opts, argc, argv);
		BTK_CHECK_NEG(r);

		r = btk_utxodb_main(opts);
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "addressdb") == 0)
	{
		opts = malloc(sizeof(*opts));
		ERROR_CHECK_FALSE(opts, "Memory allocation error.");
		
		r = opts_get(opts, argc, argv);
		BTK_CHECK_NEG(r);

		r = btk_addressdb_main(opts);
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "version") == 0)
	{
		r = btk_version_main();
		BTK_CHECK_NEG(r);
	}
	else if (strcmp(command, "help") == 0)
	{
		if (argc > 2)
		{
			r = btk_help_main(argv[2]);
		}
		else
		{
			r = btk_help_main(NULL);
		}
		BTK_CHECK_NEG(r);
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("'%s' is not a valid command.", command);
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

