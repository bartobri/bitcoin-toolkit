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
#include "mods/error.h"

#define OUTPUT_ADDRESS          1
#define OUTPUT_BECH32_ADDRESS   2
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

#define OUTPUT_BUFFER           150

int btk_vanity_main(int argc, char *argv[])
{
	int i, k, o, r, row;
	time_t current, start;
	long int estimate;
	PubKey key = NULL;
	PrivKey priv = NULL;
	char *input;
	int input_len;
	char pubkey_str[OUTPUT_BUFFER];
	char privkey_str[OUTPUT_BUFFER];

	int input_insensitive  = FALSE;
	int output_format      = OUTPUT_ADDRESS;
	int output_compression = FALSE;
	int output_testnet     = FALSE;
	
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
					error_log("Unknown option '-%c'.\n", optopt);
				}
				else
				{
					error_log ("Unknown option character '\\x%x'.\n", optopt);
				}
				return -1;
		}
	}

	r = input_get_str(&input);
	if (r < 0)
	{
		error_log("Could not get input.");
		return -1;
	}

	input_len = r;
	if (input_len > 10)
	{
		error_log("Match string is too long. This program only supports 10 characters or less.");
		return -1;
	}

	switch (output_format)
	{
		case OUTPUT_ADDRESS:
			for (i = 0; i < input_len; ++i)
			{
				if ((input_insensitive && !base58_ischar(toupper(input[i])) && !base58_ischar(tolower(input[i]))) || (!input_insensitive && !base58_ischar(input[i])))
				{
					error_log("Invalid characters in match string. Must only contain base58 characters");
					return -1;
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
					error_log("Could not calculate bech32 address.");
					return -1;
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

	if (output_testnet)
	{
		network_set_test();
	}

	// Getting cursor row
	if (!isatty(STDIN_FILENO) && !freopen ("/dev/tty", "r", stdin))
	{
		error_log("Terminal error. Cannot associate STDIN with terminal.");
		return -1;
	}
	btktermio_init_terminal();
	row = btktermio_get_cursor_row();
	btktermio_restore_terminal();

	i = 0;
	start = time(NULL);

	// Start searching
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

		priv = malloc(privkey_sizeof());
		if (priv == NULL)
		{
			error_log("Memory allocation error");
			return -1;
		}

		r = privkey_new(priv);
		if (r < 0)
		{
			error_log("Could not generate a new private key.");
			return -1;
		}
	
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

		key = malloc(pubkey_sizeof());
		if (key == NULL)
		{
			error_log("Memory allocation error");
			return -1;
		}

		r = pubkey_get(key, priv);
		if (r < 0)
		{
			error_log("Could not calculate new public key.");
			return -1;
		}

		r = privkey_to_wif(privkey_str, priv);
		if (r < 0)
		{
			error_log("Could not convert private key to WIF format.");
			return -1;
		}
		if (output_format == OUTPUT_ADDRESS)
		{
			r = pubkey_to_address(pubkey_str, key);
			if (r < 0)
			{
				error_log("Could not calculate public key address.");
				return -1;
			}
		}
		else if (output_format == OUTPUT_BECH32_ADDRESS)
		{
			if(!pubkey_is_compressed(key))
			{
				error_log("Bech32 addresses cannot be uncompressed.");
				return -1;
			}
			r = pubkey_to_bech32address(pubkey_str, key);
			if (r < 0)
			{
				error_log("Could not calculate bech32 public key address.");
				return -1;
			}
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
						if (pubkey_str[k+1] != toupper(input[k]) && pubkey_str[k+1] != tolower(input[k]))
						{
							break;
						}
					}
					if (k == input_len)
					{
						printf("\nVanity Address Found!\nPrivate Key: %s\nAddress:     %s\n", privkey_str, pubkey_str);
						return 1;
					}
				}
				else
				{
					if (strncmp(input, pubkey_str + 1, input_len) == 0)
					{
						printf("\nVanity Address Found!\nPrivate Key: %s\nAddress:     %s\n", privkey_str, pubkey_str);
						return 1;
					}
				}
				break;
			case OUTPUT_BECH32_ADDRESS:
				if(!pubkey_is_compressed(key))
				{
					error_log("Bech32 addresses cannot be uncompressed.");
					return -1;
				}

				if (strncmp(input, pubkey_str + 4, input_len) == 0)
				{
					printf("\nVanity address found!\nPrivate Key: %s\nAddress:     %s\n", privkey_str, pubkey_str);
					return 1;
				}
				break;
		}

		free(priv);
		free(key);
	}

	free(input);

	return 1;
}