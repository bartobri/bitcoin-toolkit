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

/*
 * Flags
 */
static int flag_input_new = 0;
static int flag_output_compressed = 0;
static int flag_output_uncompressed = 0;
static int flag_format_newline = 0;

int btk_privkey_init(int argc, char *argv[]) {
	int o;
	PrivKey key;
	
	// Check arguments
	while ((o = getopt(argc, argv, "nCUN")) != -1) {
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
	
	// Process Input flags
	if (flag_input_new) {
		key = privkey_new();
	} else {
		key = privkey_new();
	}
	
	// Process Output Flags
	if (flag_output_compressed) {
		printf("%s", privkey_to_wif(privkey_compress(key)));
	} else if (flag_output_uncompressed) {
		printf("%s", privkey_to_wif(privkey_uncompress(key)));
	} else {
		printf("%s", privkey_to_wif(privkey_compress(key)));
	}
	
	// Process format flags
	if (flag_format_newline)
			printf("\n");

	privkey_free(key);

	return EXIT_SUCCESS;
}
