/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define INPUT_BUFFER_SIZE    (PUBKEY_UNCOMPRESSED_LENGTH * 2) + 2

/*
 * Static Function Declarations
 */
static int btk_keypair_read_input(void);

/*
 * Static Variable Declarations
 */
static unsigned char input_buffer[INPUT_BUFFER_SIZE];
static int flag_input_new = 0;
static int flag_format_newline = 0;

int btk_keypair_main(int argc, char *argv[]) {
	int o;
	PrivKey priv = NULL;
	PubKey pub = NULL;
	
	// Check arguments
	while ((o = getopt(argc, argv, "nN")) != -1) {
		switch (o) {
			// Input flags
			case 'n':
				flag_input_new = 1;
				break;

			// Output flags

			// Format flags
			case 'N':
				flag_format_newline = 1;
				break;

			case '?':
				if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return EXIT_FAILURE;
		}
	}
	
	// Set default input flag if none specified
	if (!flag_input_new)
		flag_input_new = 1;
	
	// Process input flags
	if (flag_input_new) {
		priv = privkey_new();
	}

	// Check that we have keys
	assert(priv);

	// Process output flags
	

	// Process format flags
	if (flag_format_newline)
			printf("\n");
	
	privkey_free(priv);
	pubkey_free(pub);

	return EXIT_SUCCESS;
}

static int btk_keypair_read_input(void) {
	int i, c;

	for (i = 0; i < INPUT_BUFFER_SIZE && (c = getchar()) != EOF; ++i)
		input_buffer[i] = c;

	return i;
}
