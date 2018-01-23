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
#include "mods/base58.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define INPUT_BUFFER_SIZE    PRIVKEY_LENGTH * 2

/*
 * Static Function Declarations
 */
static int btk_privkey_read_input(void);

/*
 * Static Variable Declarations
 */
static unsigned char input_buffer[INPUT_BUFFER_SIZE];
static int flag_input_new = 0;
static int flag_input_hex = 0;
static int flag_input_raw = 0;
static int flag_input_wif = 0;
static int flag_output_raw = 0;
static int flag_output_hex = 0;
static int flag_output_compressed = 0;
static int flag_output_uncompressed = 0;
static int flag_format_newline = 0;

int btk_privkey_main(int argc, char *argv[]) {
	int o;
	PrivKey key = NULL;
	
	// Check arguments
	while ((o = getopt(argc, argv, "nhrwCUHRN")) != -1) {
		switch (o) {
			// Input flags
			case 'n':
				flag_input_new = 1;
				break;
			case 'h':
				flag_input_hex = 1;
				break;
			case 'r':
				flag_input_raw = 1;
				break;
			case 'w':
				flag_input_wif = 1;
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
			case 'R':
				flag_output_raw = 1;
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
	
	// Set default input flag if none specified
	if (!flag_input_new && !flag_input_hex && !flag_input_raw && !flag_input_wif)
		flag_input_new = 1;
	
	// Process Input flags
	if (flag_input_new) {
		key = privkey_new();
	} else if (flag_input_hex) {
		int i, cnt;
		cnt = btk_privkey_read_input();
		if (cnt < PRIVKEY_LENGTH * 2) {
			fprintf(stderr, "Error: Invalid input.\n");
			return EXIT_FAILURE;
		}
		for (i = 0; i < cnt; ++i) {
			if ((input_buffer[i] < 'A' || input_buffer[i] > 'F') && (input_buffer[i] < '0' || input_buffer[i] > '9') && (input_buffer[i] < 'a' || input_buffer[i] > 'z')) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
		}
		key = privkey_from_hex((char *)input_buffer);
	} else if (flag_input_raw) {
		int cnt;
		cnt = btk_privkey_read_input();
		if (cnt < PRIVKEY_LENGTH) {
			fprintf(stderr, "Error: Invalid input.\n");
			return EXIT_FAILURE;
		}
		key = privkey_from_raw(input_buffer);
	} else if (flag_input_wif) {
		int i, cnt;
		cnt = btk_privkey_read_input();
		if (cnt < 51) {
			fprintf(stderr, "Error: Invalid input.\n");
			return EXIT_FAILURE;
		}
		for (i = 0; i < cnt; ++i) {
			if (!base58_ischar(input_buffer[i])) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
		}
		input_buffer[cnt] = '\0';
		key = privkey_from_wif((char *)input_buffer);
	}
	
	// Make sure we have a key
	assert(key);

	// Set default output flag if none specified
	if (!flag_output_hex && !flag_output_compressed && !flag_output_uncompressed && !flag_output_raw)
		flag_output_compressed = 1;
	
	// Process Output Flags
	// TODO - should I be compressing hex and raw outputs?
	if (flag_output_hex) {
		printf("%s", privkey_to_hex(privkey_uncompress(key)));
	} else if (flag_output_compressed) {
		printf("%s", privkey_to_wif(privkey_compress(key)));
	} else if (flag_output_uncompressed) {
		printf("%s", privkey_to_wif(privkey_uncompress(key)));
	} else if (flag_output_raw) {
		int i;
		unsigned char *r;
		r = privkey_to_raw(privkey_uncompress(key));
		for (i = 0; i < PRIVKEY_LENGTH; ++i) {
			printf("%c", r[i]);
		}
		FREE(r);
	}
	
	// Process format flags
	if (flag_format_newline)
			printf("\n");

	privkey_free(key);

	return EXIT_SUCCESS;
}

static int btk_privkey_read_input(void) {
	int i, c;

	for (i = 0; i < INPUT_BUFFER_SIZE && (c = getchar()) != EOF; ++i)
		input_buffer[i] = c;

	return i;
}

void btk_privkey_help(void) {
	fprintf(stderr, "NAME:\n");
	fprintf(stderr, "       btk privkey - Generate and/or modify the format of a private key.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "USAGE:\n");
	fprintf(stderr, "       btk privkey [-n | -h | -r | -w]\n");
	fprintf(stderr, "                   [-C | -U | -H | -R]\n");
	fprintf(stderr, "                   [-N]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "DESCRIPTION:\n");
	fprintf(stderr, "       The privkey command displays private keys in the format that you specify. The source for the private key can either be your local CSPRNG, in which case a new private key is generated, or it can be from input that you supply. Allowed input formats are hex, raw (unsigned char string), or WIF (compressed or uncompressed).\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       Output formats are similar to iput formats. Private keys can be displayed as WIF compressed, WIF uncompressed, hex, or raw.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       With the privkey command you can generate new private keys in any supported format, or you can modify the format of existing private keys by supplying it as input and then specifying the desired output format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "INPUT OPTIONS\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -n (default)\n");
	fprintf(stderr, "           Generate a new private key from your local CSPRNG.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -h\n");
	fprintf(stderr, "           Supply a private key as input in hexidecimal format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -r\n");
	fprintf(stderr, "           Supply a private key as input in raw format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -w\n");
	fprintf(stderr, "           Supply a private key as input in WIF format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "OUTPUT OPTIONS\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -C (default)\n");
	fprintf(stderr, "           Display the private key in compressed WIF format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -U\n");
	fprintf(stderr, "           Display the private key in uncompressed WIF format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -H\n");
	fprintf(stderr, "           Display the private key in hexidecimal format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -R\n");
	fprintf(stderr, "           Display the private key in raw format.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "MISC OPTIONS\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       -N\n");
	fprintf(stderr, "           Display a newline character after the private key.\n");
	fprintf(stderr, "\n");
}



