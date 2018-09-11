/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "ctrl_mods/btk_help.h"
#include "ctrl_mods/btk_privkey.h"
#include "ctrl_mods/btk_pubkey.h"
#include "ctrl_mods/btk_vanity.h"
#include "ctrl_mods/btk_node.h"
#include "mods/mem.h"

int main(int argc, char *argv[])
{
	unsigned char* input = NULL;
	size_t input_len = 0;

	if (argc <= 1)
	{
		btk_help_help();
		return EXIT_FAILURE;
	}

	if (!isatty(fileno(stdin)))
	{
		while (1)
		{
			if (ioctl(STDIN_FILENO, FIONREAD, &input_len) >= 0 && input_len > 0)
			{
				input = ALLOC(input_len);
				read(STDIN_FILENO, input, input_len);
				break;
			}
		}
	}
	
	if (strcmp(argv[1], "help") == 0)
	{
		return btk_help_main(argc, argv);
	}
	else if (strcmp(argv[1], "privkey") == 0)
	{
		return btk_privkey_main(argc, argv, input, input_len);
	}
	else if (strcmp(argv[1], "pubkey") == 0)
	{
		return btk_pubkey_main(argc, argv, input, input_len);
	}
	else if (strcmp(argv[1], "vanity") == 0)
	{
		return btk_vanity_main(argc, argv, input, input_len);
	}
	else if (strcmp(argv[1], "node") == 0)
	{
		return btk_node_main(argc, argv, input, input_len);
	}
	else
	{
		fprintf(stderr, "Invalid Command: %s\n", argv[1]);
		btk_help_help();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
