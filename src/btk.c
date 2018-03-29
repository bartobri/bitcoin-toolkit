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

int main(int argc, char *argv[]) {
	char *cmd;

	if (argc <= 1) {
		btk_help_help();
		return EXIT_FAILURE;
	}

	cmd = argv[1];

	argv[1] = argv[0];
	argv++;
	argc--;
	
	if (strcmp(cmd, "help") == 0) {
		return btk_help_main(argc, argv);
	} else if (strcmp(cmd, "privkey") == 0) {
		return btk_privkey_main(argc, argv);
	} else if (strcmp(cmd, "pubkey") == 0) {
		return btk_pubkey_main(argc, argv);
	} else {
		fprintf(stderr, "Unknown Command: %s\n", cmd);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
