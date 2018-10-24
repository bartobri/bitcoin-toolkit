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
#include "mods/error.h"

int btk_help_main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		btk_help_commands();
		return 1;
	}

	if (strcmp(argv[2], "privkey") == 0)
	{
		printf("privkey help here\n");
	}
	else if (strcmp(argv[2], "pubkey") == 0)
	{
		printf("pubkey help here\n");
	}
	else if (strcmp(argv[2], "node") == 0)
	{
		printf("node help here\n");
	}
	else if (strcmp(argv[2], "vanity") == 0)
	{
		printf("vanity help here\n");
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Invalid command.");
		return -1;
	}
	
	return 1;
}

void btk_help_commands(void)
{
	printf("usage: btk <command> [<args>]\n");
	printf("\n");
	printf("Here is a list of btk commands.\n");
	printf("\n");
	printf("   privkey      Create and/or manipulate private keys.\n");
	printf("   pubkey       Create and/or manipulate public keys and addresses.\n");
	printf("   vanity       Generate a vanity address.\n");
	printf("   node         Interface with bitcoin nodes.\n");
	printf("   version      Print btk version.\n");
	printf("\n");
	printf("See 'btk help <command>' to read more about a specific command.\n");
}
