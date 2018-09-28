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
#include <time.h>
#include <gmp.h>
#include <sys/ioctl.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/network.h"
#include "mods/base58.h"
#include "mods/base32.h"
#include "mods/btktermio.h"
#include "mods/input.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define OUTPUT_ADDRESS          1
#define OUTPUT_BECH32_ADDRESS   2
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

int btk_vanity_main(int argc, char *argv[])
{
	int o, r, row;
	size_t i, k;
	time_t current, start;
	long int estimate;
	PubKey key = NULL;
	PrivKey priv = NULL;
	char *pubkey_str, *privkey_str;
	unsigned char *input;
	size_t input_len;

	// Default flags
	int input_insensitive  = FALSE;
	int output_format      = OUTPUT_ADDRESS;
	int output_compression = FALSE;
	int output_testnet     = FALSE;
	
	// Check arguments
	while ((o = getopt(argc, argv, "iABCUT")) != -1)
	{
		switch (o)
		{

			// Insensitive Search
			case 'i':
				input_insensitive = TRUE;
				break;

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
				{
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				}
				else
				{
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				}
				return EXIT_FAILURE;
		}
	}

	// Get input
	input_len = input_get(&input);

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
				if ((input_insensitive && !base58_ischar(toupper(input[i])) && !base58_ischar(tolower(input[i]))) || (!input_insensitive && !base58_ischar(input[i])))
				{
					fprintf(stderr, "Match string must only contain base58 characters\n");
					return EXIT_FAILURE;
				}
			}
			break;
		case OUTPUT_BECH32_ADDRESS:
			// If we are executing a case insensitive search for a bech32 address,
			// Just convert all uppercase letters to lowercase and performs a regular
			// case sensitive search, since bech32 has no uppercase letters.
			if (input_insensitive)
			{
				for (i = 0; i < input_len; ++i)
				{
					if (input[i] >= 'A' && input[i] <= 'Z')
					{
						input[i] = tolower(input[i]);
					}
				}
			}
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

	// Estimate time
	switch (output_format)
	{
		case OUTPUT_ADDRESS:
			if (!input_insensitive)
			{
				estimate = 58;
				for (i = 1; i < input_len; ++i)
				{
					estimate *= 58;
				}
			}
			else
			{
				if (isalpha(input[0]))
				{
					estimate = 34;
				}
				else
				{
					estimate = 58;
				}
				for (i = 1; i < input_len; ++i)
				{
					if (isalpha(input[i]))
					{
						estimate *= 34;
					}
					else
					{
						estimate *= 58;
					}
				}
			}
			break;
		case OUTPUT_BECH32_ADDRESS:
			estimate = 32;
			for (i = 1; i < input_len; ++i)
			{
				estimate *= 32;
			}
			break;
	}

	// Process testnet option
	if (output_testnet)
	{
		network_set_test();
	}

	// Getting cursor row
	if (!isatty(STDIN_FILENO) && !freopen ("/dev/tty", "r", stdin))
	{
		fprintf(stderr, "Error. Can't associate STDIN with terminal.\n");
		return -1;
	}
	btktermio_init_terminal();
	row = btktermio_get_cursor_row();
	btktermio_restore_terminal();

	i = 0;
	start = time(NULL);

	// Allocate memory for the public/private strings
	privkey_str = ALLOC(PRIVKEY_WIF_LENGTH_MAX + 1);

	while (1)
	{
		if (row >= 0)
		{
			btktermio_move_cursor(row, 0);
			if (i == 0)
			{
				printf("Searching...");
				fflush(stdout);
				btktermio_move_cursor(row, 0);
			}
		}
		else
		{
			printf("\n");
		}

		priv = ALLOC(privkey_sizeof());
		r = privkey_new(priv);
		if (r < 0)
		{
			fprintf(stderr, "Error: Unable to generate new private key.\n");
			return EXIT_FAILURE;
		}
	
		// Set output compression only if the option is set. Otherwise,
		// compression is based on input.
		switch (output_compression)
		{
			case FALSE:
				break;
			case OUTPUT_COMPRESS:
				privkey_compress(priv);
				break;
			case OUTPUT_UNCOMPRESS:
				privkey_uncompress(priv);
				break;
		}

		// Get public key from private key
		key = ALLOC(pubkey_sizeof());
		r = pubkey_get(key, priv);
		if (r < 0)
		{
			fprintf(stderr, "Error: Unable to calculate public key from private key.");
			return EXIT_FAILURE;
		}

		// Get key strings
		privkey_to_wif(privkey_str, priv);
		if (output_format == OUTPUT_ADDRESS)
		{
			pubkey_str = ALLOC(35);
			r = pubkey_to_address(pubkey_str, key);
		}
		else if (output_format == OUTPUT_BECH32_ADDRESS)
		{
			if(!pubkey_is_compressed(key))
			{
				fprintf(stderr, "Error: Can not use an uncompressed private key for a bech32 address.\n");
				return EXIT_FAILURE;
			}
			pubkey_str = ALLOC(43);
			r = pubkey_to_bech32address(pubkey_str, key);
		}

		// Track time and print status
		current = time(NULL);
		++i;
		if (current - start != 0) {
			printf("%-45s Estimated Seconds: %ld of %ld", pubkey_str, current - start, (estimate / (i / (current - start))));
			fflush(stdout);
		}
	
		// Process output
		switch (output_format)
		{
			case OUTPUT_ADDRESS:
				if (input_insensitive)
				{
					for (k = 0; k < input_len; ++k)
					{
						if (pubkey_str[k+1] != toupper((int)input[k]) && pubkey_str[k+1] != tolower((int)input[k]))
						{
							break;
						}
					}
					if (k == input_len)
					{
						printf("\nVanity Address Found!\nPrivate Key: %s\nAddress:     %s\n", privkey_str, pubkey_str);
						return EXIT_SUCCESS;
					}
				}
				else
				{
					if (strncmp((char *)input, pubkey_str + 1, input_len) == 0)
					{
						printf("\nVanity Address Found!\nPrivate Key: %s\nAddress:     %s\n", privkey_str, pubkey_str);
						return EXIT_SUCCESS;
					}
				}
				break;
			case OUTPUT_BECH32_ADDRESS:
				if(!pubkey_is_compressed(key))
				{
					fprintf(stderr, "Error: Can not use an uncompressed private key for a bech32 address.\n");
					return EXIT_FAILURE;
				}

				if (strncmp((char *)input, pubkey_str + 4, input_len) == 0)
				{
					printf("\nVanity address found!\nPrivate Key: %s\nAddress:     %s\n", privkey_str, pubkey_str);
					return EXIT_SUCCESS;
				}
				break;
		}

		// Free allocated memory
		FREE(priv);
		FREE(key);
		FREE(pubkey_str);
	}
	
	FREE(privkey_str);

	return EXIT_SUCCESS;
}