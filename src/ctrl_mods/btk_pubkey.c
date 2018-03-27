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
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "mods/privkey.h"
#include "mods/network.h"
#include "mods/pubkey.h"
#include "mods/base58.h"
#include "mods/base58check.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define BUFFER_SIZE             1000
#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_RAW               3
#define INPUT_STR               4
#define INPUT_DEC               5
#define INPUT_GUESS             6
#define OUTPUT_ADDRESS          1
#define OUTPUT_BECH32_ADDRESS   2
#define OUTPUT_HEX              3
#define OUTPUT_RAW              4
#define TRUE                    1
#define FALSE                   0

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
	int input_format       = INPUT_GUESS;
	int output_format      = OUTPUT_ADDRESS;
	int output_newline     = FALSE;
	int output_testnet     = FALSE;

	// Zero the input buffer
	memset(input_buffer, 0, BUFFER_SIZE);
	
	// Check arguments
	while ((o = getopt(argc, argv, "whrsdABHRNT")) != -1) {
		switch (o) {
			// Input format
			case 'w':
				input_format = INPUT_WIF;
				break;
			case 'h':
				input_format = INPUT_HEX;
				break;
			case 'r':
				input_format = INPUT_RAW;
				break;
			case 's':
				input_format = INPUT_STR;
				break;
			case 'd':
				input_format = INPUT_DEC;
				break;

			// Output format
			case 'A':
				output_format = OUTPUT_ADDRESS;
				break;
			case 'B':
				output_format = OUTPUT_BECH32_ADDRESS;
				break;
			case 'H':
				output_format = OUTPUT_HEX;
				break;
			case 'R':
				output_format = OUTPUT_RAW;
				break;

			// Other options
			case 'N':
				output_newline = TRUE;
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

	// Process testnet option
	switch (output_testnet) {
		case FALSE:
			break;
		case TRUE:
			network_set_test();
	}
	
	// Process input
	switch (input_format) {
		case INPUT_WIF:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);

			// Ignore white spaces at the end of input
			while (isspace((int)input_buffer[c-1]))
				--c;

			for (i = 0; i < c; ++i)
				if (!base58_ischar(input_buffer[i]))
					break;
			if (i < PRIVKEY_WIF_LENGTH_MIN) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			input_buffer[i] = '\0';
			if (!base58check_valid_checksum((char *)input_buffer, strlen((char *)input_buffer))) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			priv = privkey_from_wif((char *)input_buffer);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_HEX:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);

			// Ignore white spaces at the end of input
			while (isspace((int)input_buffer[c-1]))
				--c;

			for (i = 0; i < c; ++i)
				if (!hex_ischar(input_buffer[i]))
					break;
			if (i < PRIVKEY_LENGTH * 2) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			if (i == PRIVKEY_LENGTH * 2 + 1) {           // Incomplete compression flag
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			input_buffer[i] = '\0';
			priv = privkey_from_hex((char *)input_buffer);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_RAW:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
			if (c < PRIVKEY_LENGTH) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			priv = privkey_from_raw(input_buffer, c);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_STR:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);

			// Ignore newline at the end of input
			if((int)input_buffer[c-1] == '\n')
				--c;

			input_buffer[c] = '\0';
			priv = privkey_from_str((char *)input_buffer);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_DEC:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);

			// Ignore white spaces at the end of input
			while (isspace((int)input_buffer[c-1]))
				--c;

			for (i = 0; i < c; ++i) {
				if (input_buffer[i] < '0' || input_buffer[i] > '9') {
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}

			input_buffer[i] = '\0';
			priv = privkey_from_dec((char *)input_buffer);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
		case INPUT_GUESS:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
			priv = privkey_from_guess(input_buffer, c);
			key = pubkey_get(priv);
			privkey_free(priv);
			break;
	}
	
	// Ensure we have a key
	if (!key) {
		fprintf(stderr, "Unable to generate public key. Input required.\n");
		return EXIT_FAILURE;
	}
	
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
