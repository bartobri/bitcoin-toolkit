/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btk_help.h"
#include "btk_privkey.h"
#include "mods/error.h"

int btk_help_main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		btk_help_help();
		return -1;
	}

	if (strcmp(argv[2], "privkey") == 0)
	{
		fprintf(stderr, "privkey man page here\n");
	}
	else if (strcmp(argv[2], "pubkey") == 0)
	{
		fprintf(stderr, "pubkey man page here\n");
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Invalid command.");
		return -1;
	}
	
	return 1;
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
