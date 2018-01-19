/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/btk_privkey.h"
#include "libs/btk_pubkey.h"
#include "libs/btk_keypair.h"

// Function prototypes
void show_help(void);

int main(int argc, char *argv[]) {

	if (argc <= 1) {
		show_help();
		return EXIT_FAILURE;
	}
	
	if (strcmp(argv[1], "privkey") == 0) {
		return btk_privkey_main(argc, argv);
	} else if (strcmp(argv[1], "pubkey") == 0) {
		return btk_pubkey_main(argc, argv);
	} else if (strcmp(argv[1], "keypair") == 0) {
		return btk_keypair_main(argc, argv);
	} else {
		fprintf(stderr, "Unknown Command: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void show_help(void) {
	fprintf(stderr, "usage: btk <command> [<args>]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Here is a list of btk commands.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   privkey      Generate and/or format private keys\n");
	fprintf(stderr, "   pubkey       Generate and/or format public keys\n");
	fprintf(stderr, "   keypair      Generate and/or format key pairs\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Run 'btk help <command>' to read about the command and see a list of available options.\n");
}
