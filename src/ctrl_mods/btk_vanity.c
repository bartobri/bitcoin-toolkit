/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <gmp.h>
#include <sys/ioctl.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/network.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define OUTPUT_ADDRESS          1
#define OUTPUT_BECH32_ADDRESS   2
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

static size_t btk_vanity_get_input(unsigned char** output);

int btk_vanity_main(int argc, char *argv[], unsigned char *input, size_t input_len) {
	int o;
	PubKey key = NULL;
	PrivKey priv = NULL;

	// Default flags
	int output_format      = OUTPUT_ADDRESS;
	int output_compression = FALSE;
	int output_testnet     = FALSE;
	
	// Check arguments
	while ((o = getopt(argc, argv, "ABCUT")) != -1) {
		switch (o) {

			// Output format
			case 'A':
				output_format = OUTPUT_ADDRESS;
				break;
			case 'B':
				output_format = OUTPUT_BECH32_ADDRESS;
				break;

				// Output Compression
			case 'C':
				output_compression = OUTPUT_COMPRESS;
				break;
			case 'U':
				output_compression = OUTPUT_UNCOMPRESS;
				break;

			// Testnet Option
			case 'T':
				output_testnet = TRUE;
				break;

			// Unknown option
			case '?':
				if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return EXIT_FAILURE;
		}
	}

	// Get input if we need to
	if (input == NULL)
		input_len = btk_vanity_get_input(&input);

	// Process testnet option
	switch (output_testnet) {
		case FALSE:
			break;
		case TRUE:
			network_set_test();
	}

	while (1)
	{
		priv = privkey_new();
		assert(priv);
	
		// Set output compression only if the option is set. Otherwise,
		// compression is based on input.
		switch (output_compression) {
			case FALSE:
				break;
			case OUTPUT_COMPRESS:
				priv = privkey_compress(priv);
				break;
			case OUTPUT_UNCOMPRESS:
				priv = privkey_uncompress(priv);
				break;
		}

		// Get public key from private key
		key = pubkey_get(priv);

		// Print private key
		printf("%s ", privkey_to_wif(priv));
	
		// Process output
		switch (output_format) {
			case OUTPUT_ADDRESS:
				printf("%s", pubkey_to_address(key));
				break;
			case OUTPUT_BECH32_ADDRESS:
				if(!pubkey_is_compressed(key)) {
					fprintf(stderr, "Error: Can not use an uncompressed private key for a bech32 address.\n");
					return EXIT_FAILURE;
				}
				printf("%s", pubkey_to_bech32address(key));
				break;
		}
		printf("\n");
	}

	// Free allocated memory
	privkey_free(priv);
	pubkey_free(key);

	return EXIT_SUCCESS;
}

static size_t btk_vanity_get_input(unsigned char** output) {
	size_t i, size, increment = 100;
	int o;

	size = increment;

	*output = ALLOC(size);

	for (i = 0; (o = getchar()) != '\n'; ++i)
		{
		if (i == size)
			{
			size += increment;
			RESIZE(*output, size);
			}
		(*output)[i] = (unsigned char)o;
		}

	return i;
}