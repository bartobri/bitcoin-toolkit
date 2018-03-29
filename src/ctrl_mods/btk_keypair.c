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

/*
 * Static Variable Declarations
 */
static int flag_input_new = 0;
static int flag_output_compressed = 0;
static int flag_output_uncompressed = 0;
static int flag_output_hex = 0;
static int flag_format_newline = 0;
static char *delim = " ";

int btk_keypair_main(int argc, char *argv[]) {
	int o;
	PrivKey priv = NULL;
	PubKey pub = NULL;
	
	// Check arguments
	while ((o = getopt(argc, argv, "nCUHND:")) != -1) {
		switch (o) {
			// Input flags
			case 'n':
				flag_input_new = 1;
				break;

			// Output flags
			case 'C':
				flag_output_compressed = 1;
				break;
			case 'U':
				flag_output_uncompressed = 1;
				break;
			case 'H':
				flag_output_hex = 1;
				break;

			// Format flags
			case 'N':
				flag_format_newline = 1;
				break;
			case 'D':
				delim = optarg;
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
	
	// Set default output flag if none specified
	if (!flag_output_compressed && !flag_output_uncompressed && !flag_output_hex)
		flag_output_compressed = 1;

	// Process output flags
	if (flag_output_compressed) {
		priv = privkey_compress(priv);
		pub = pubkey_get(priv);
		printf("%s%s%s", privkey_to_wif(priv), delim, pubkey_to_address(pub));
	} else if (flag_output_uncompressed) {
		priv = privkey_uncompress(priv);
		pub = pubkey_get(priv);
		printf("%s%s%s", privkey_to_wif(priv), delim, pubkey_to_address(pub));
	} else if (flag_output_hex) {
		priv = privkey_compress(priv);
		pub = pubkey_get(priv);
		printf("%s%s%s", privkey_to_hex(priv), delim, pubkey_to_hex(pub));
	}

	// Process format flags
	if (flag_format_newline)
			printf("\n");
	
	privkey_free(priv);
	pubkey_free(pub);

	return EXIT_SUCCESS;
}