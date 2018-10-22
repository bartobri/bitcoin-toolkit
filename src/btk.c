/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctrl_mods/btk_help.h"
#include "ctrl_mods/btk_privkey.h"
#include "ctrl_mods/btk_pubkey.h"
#include "ctrl_mods/btk_vanity.h"
#include "ctrl_mods/btk_node.h"
#include "ctrl_mods/btk_version.h"
#include "mods/error.h"

#define MAX_COMMAND_LEN 100

int main(int argc, char *argv[])
{
	int i, r;
	char command[MAX_COMMAND_LEN];

	r = 0;

	if (argc <= 1)
	{
		btk_help_help();
		return EXIT_FAILURE;
	}

	// Assembling the original command for logging purposes before we
	// pass the args to the control modules.
	memset(command, 0, MAX_COMMAND_LEN);
	for (i = 0; i < argc; ++i)
	{
		if (i > 0)
		{
			strncat(command, " ", MAX_COMMAND_LEN - 1 - strlen(command));
		}
		strncat(command, argv[i], MAX_COMMAND_LEN - 1 - strlen(command));
	}
	
	if (strcmp(argv[1], "help") == 0)
	{
		r = btk_help_main(argc, argv);
	}
	else if (strcmp(argv[1], "privkey") == 0)
	{
		r = btk_privkey_main(argc, argv);
	}
	else if (strcmp(argv[1], "pubkey") == 0)
	{
		r = btk_pubkey_main(argc, argv);
	}
	else if (strcmp(argv[1], "vanity") == 0)
	{
		r = btk_vanity_main(argc, argv);
	}
	else if (strcmp(argv[1], "node") == 0)
	{
		r = btk_node_main(argc, argv);
	}
	else if (strcmp(argv[1], "version") == 0)
	{
		r = btk_version_main(argc, argv);
	}
	else
	{
		error_log("Error [%s]:", command);
		error_log("Invalid Command: %s\n", argv[1]);
		error_print();
		btk_help_help();
		return EXIT_FAILURE;
	}

	if (r < 0)
	{
		error_log("Error [%s]:", command);
		error_print();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
