/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btk_help.h"
#include "btk_privkey.h"

int btk_help_main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		btk_help_help();
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "help") == 0)
	{
		btk_help_help();
	}
	else if (strcmp(argv[1], "privkey") == 0)
	{
		fprintf(stderr, "privkey man page here\n");
	}
	else if (strcmp(argv[1], "pubkey") == 0)
	{
		fprintf(stderr, "pubkey man page here\n");
	}
	else
	{
		fprintf(stderr, "Unknown Command: %s\n", argv[1]);
		fprintf(stderr, "Run 'btk help' for a list of valid commands.\n");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

void btk_help_help(void)
{
	fprintf(stderr, "usage: btk <command> [<args>]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Here is a list of btk commands.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   privkey      Generate and/or format private keys\n");
	fprintf(stderr, "   pubkey       Generate and/or format public keys\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Run 'btk help <command>' to read about the command and see a list of available options.\n");
}
