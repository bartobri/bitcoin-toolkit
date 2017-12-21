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
static int flag_format_newline = 0;

int btk_privkey_init(int argc, char *argv[]) {
	int o;
	PrivKey key;
	
	// Check arguments
	while ((o = getopt(argc, argv, "nRN")) != -1) {
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
	
	// Check for minimum flags
	if (!flag_input_new) {
		fprintf(stderr, "Missing source flag.\n");
		return EXIT_FAILURE;
	}
	
	// Execute flags
	if (flag_input_new) {
		key = privkey_new_compressed();
		printf("%s", privkey_to_wif(key));

		if (flag_format_newline)
			printf("\n");
	}
	
	return EXIT_SUCCESS;
}
