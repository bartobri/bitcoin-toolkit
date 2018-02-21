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
#include "mods/base58.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define BUFFER_SIZE             1000
#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_RAW               3
#define OUTPUT_ADDRESS          1
#define OUTPUT_HEX              2
#define OUTPUT_RAW              3
#define TRUE                    1
#define FALSE                   0

/*
 * Static Function Declarations
 */
static int btk_pubkey_read_input(void);

/*
 * Static Variable Declarations
 */
static unsigned char input_buffer[BUFFER_SIZE];

int btk_pubkey_main(int argc, char *argv[]) {
	int o;
	PubKey key = NULL;
	PrivKey priv;
	size_t i, c;
	unsigned char *t;

	// Default flags
	int input_format       = FALSE;
	int output_format      = OUTPUT_ADDRESS;
	int output_newline     = FALSE;
	
	// Check arguments
	while ((o = getopt(argc, argv, "whrAHRN")) != -1) {
		switch (o) {
			// Input flags
			case 'w':
				input_format = INPUT_WIF;
				break;
			case 'h':
				input_format = INPUT_HEX;
				break;
			case 'r':
				input_format = INPUT_RAW;
				break;

			// Output flags
			case 'A':
				output_format = OUTPUT_ADDRESS;
				break;
			case 'H':
				output_format = OUTPUT_HEX;
				break;
			case 'R':
				output_format = OUTPUT_RAW;
				break;

			// Format flags
			case 'N':
				output_newline = TRUE;
				break;

			case '?':
				if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return EXIT_FAILURE;
		}
	}
	
	// Process input
	switch (input_format) {
		case INPUT_WIF:
			c = btk_pubkey_read_input();
			// TODO - make '51' globally defined value so it can be used here and in btk_privkey
			if (c < PRIVKEY_WIF_LENGTH_MIN) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			// TODO - this doesn't gracefully handle white space chars at the end of the WIF input
			for (i = 0; i < c; ++i) {
				if (!base58_ischar(input_buffer[i])) {
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}
			input_buffer[c] = '\0';
			priv = privkey_from_wif((char *)input_buffer);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_HEX:
			c = btk_pubkey_read_input();
			if (c < PRIVKEY_LENGTH * 2) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			for (i = 0; i < c; ++i) {
				if (!hex_ischar(input_buffer[i])) {
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}
			priv = privkey_from_hex((char *)input_buffer);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_RAW:
			c = btk_pubkey_read_input();
			if (c < PRIVKEY_LENGTH) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			priv = privkey_from_raw(input_buffer, c);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		default:
			fprintf(stderr, "Error: Must specify input flag.\n");
			return EXIT_FAILURE;
	}
	
	// Ensure we have a key
	assert(key);
	
	// Process output
	switch (output_format) {
		case OUTPUT_ADDRESS:
			printf("%s", pubkey_to_address(key));
			break;
		case OUTPUT_HEX:
			printf("%s", pubkey_to_hex(key));
			break;
		case OUTPUT_RAW:
			t = pubkey_to_raw(key, &c);
			for (i = 0; i < c; ++i) {
				printf("%c", t[i]);
			}
			FREE(t);
			break;
	}

	// Process newline flag
	switch (output_newline) {
		case TRUE:
			printf("\n");
			break;
	}

	// Free allocated memory
	pubkey_free(key);

	return EXIT_SUCCESS;
}

// TODO - should this be merged with the function in btk_privkey, perhaps it's own module?
static int btk_pubkey_read_input(void) {
	int i, c;

	for (i = 0; i < BUFFER_SIZE && (c = getchar()) != EOF; ++i)
		input_buffer[i] = c;

	return i;
}
