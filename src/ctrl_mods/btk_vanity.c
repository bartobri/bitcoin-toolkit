/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <gmp.h>
#include <sys/ioctl.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/network.h"
#include "mods/base58.h"
#include "mods/base32.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define OUTPUT_ADDRESS          1
#define OUTPUT_BECH32_ADDRESS   2
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

static size_t btk_vanity_get_input(unsigned char** output);
static int btk_vanity_get_cursor_row(void);
static void btk_vanity_move_cursor(int y, int x);

int btk_vanity_main(int argc, char *argv[], unsigned char *input, size_t input_len) {
	int o, row;
	size_t i, j;
	PubKey key = NULL;
	PrivKey priv = NULL;
	char *pubkey_str;

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

	// Validate Input
	while (isspace((int)input[input_len - 1]))
	{
		--input_len;
	}
	if (input_len > 10)
	{
		fprintf(stderr, "Match string too long. Must be 10 chars or less.\n");
		return EXIT_FAILURE;
	}
	switch (output_format)
	{
		case OUTPUT_ADDRESS:
			for (i = 0; i < input_len; ++i)
			{
				if (!base58_ischar(input[i]))
				{
					fprintf(stderr, "Match string must only contain base58 characters\n");
					return EXIT_FAILURE;
				}
			}
			break;
		case OUTPUT_BECH32_ADDRESS:
			for (i = 0; i < input_len; ++i)
			{
				if (base32_get_raw(input[i]) < 0)
				{
					fprintf(stderr, "Match string must only contain bech32 characters\n");
					return EXIT_FAILURE;
				}
			}
			break;
	}

	// Process testnet option
	switch (output_testnet) {
		case FALSE:
			break;
		case TRUE:
			network_set_test();
	}

	row = btk_vanity_get_cursor_row();

	i = 0;
	j = 0;

	while (1)
	{
		if (row >= 0)
		{
			btk_vanity_move_cursor(row, 0);
			if (i == 0)
			{
				printf("Searching...");
				fflush(stdout);
				btk_vanity_move_cursor(row, 0);
			}
		}
		else
		{
			printf("\n");
		}

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
	
		// Process output
		switch (output_format) {
			case OUTPUT_ADDRESS:
				pubkey_str = pubkey_to_address(key);
				if (strncmp((char *)input, pubkey_str + 1, input_len) == 0)
				{
					printf("\nVanity Address Found!\nPrivate Key: %s\nAddress:     %s\n", privkey_to_wif(priv), pubkey_str);
					return EXIT_SUCCESS;
				}
				FREE(pubkey_str);
				break;
			case OUTPUT_BECH32_ADDRESS:
				if(!pubkey_is_compressed(key)) {
					fprintf(stderr, "Error: Can not use an uncompressed private key for a bech32 address.\n");
					return EXIT_FAILURE;
				}
				pubkey_str = pubkey_to_bech32address(key);
				if (strncmp((char *)input, pubkey_str + 4, input_len) == 0)
				{
					printf("\nVanity Address Found!\nPrivate Key: %s\nAddress:     %s\n", privkey_to_wif(priv), pubkey_str);
					return EXIT_SUCCESS;
				}
				FREE(pubkey_str);
				break;
		}

		++j;
		++i;
		if (j == 100) {
			j = 0;
			printf("%lu Addresses Searched...", i);
			fflush(stdout);
		}

		// Free allocated memory
		privkey_free(priv);
		pubkey_free(key);
	}

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

static int btk_vanity_get_cursor_row(void) {
	int i, r = 0;
	int row = 0;
	char buf[10];
	char *cmd = "\033[6n";
	struct termios tp;
	struct termios save;

	memset(buf, 0, sizeof(buf));

	if (!isatty(STDIN_FILENO) && !freopen ("/dev/tty", "r", stdin))
	{
		fprintf(stderr, "Error. Can't associate STDIN with terminal.\n");
		return -1;
	}

	if (tcgetattr(STDIN_FILENO, &tp) == -1)
	{
		return -1;
	}
	save = tp;
	tp.c_lflag &=(~ICANON & ~ECHO);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1)
	{
		return -1;
	}

	write(STDOUT_FILENO, cmd, sizeof(cmd));
	r = read(STDIN_FILENO, buf, sizeof(buf));

	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1)
	{
		return -1;
	}

	for (i = 0; i < r; ++i) {
		if (buf[i] == 27 || buf[i] == '[') {
			continue;
		}

		if (buf[i] >= '0' && buf[i] <= '9') {
			row = (row * 10) + (buf[i] - '0');
		}

		if (buf[i] == ';' || buf[i] == 'R' || buf[i] == 0) {
			break;
		}
	}

	return row;
}

static void btk_vanity_move_cursor(int y, int x) {
	printf("\033[%i;%iH", y, x);
}