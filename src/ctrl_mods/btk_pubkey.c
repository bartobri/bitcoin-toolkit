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
#include "mods/input.h"
#include "mods/error.h"

#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_RAW               3
#define INPUT_STR               4
#define INPUT_DEC               5
#define INPUT_BLOB              6
#define INPUT_GUESS             7
#define OUTPUT_ADDRESS          1
#define OUTPUT_BECH32_ADDRESS   2
#define OUTPUT_HEX              3
#define OUTPUT_RAW              4
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

int btk_pubkey_main(int argc, char *argv[])
{
	int o, r;
	PubKey key = NULL;
	PrivKey priv = NULL;
	size_t i;
	unsigned char *input;
	size_t input_len;
	char *output;
	unsigned char *uc_output;
	size_t output_len;

	// Default flags
	int input_format       = INPUT_GUESS;
	int output_format      = OUTPUT_ADDRESS;
	int output_compression = FALSE;
	int output_privkey     = FALSE;
	int output_newline     = TRUE;
	int output_testnet     = FALSE;
	
	// Check arguments
	while ((o = getopt(argc, argv, "whrsdbABHRCUPNT")) != -1)
	{
		switch (o)
		{
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
			case 'b':
				input_format = INPUT_BLOB;
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

				// Output Compression
			case 'C':
				output_compression = OUTPUT_COMPRESS;
				break;
			case 'U':
				output_compression = OUTPUT_UNCOMPRESS;
				break;

			// Other options
			case 'P':
				output_privkey = TRUE;
				break;
			case 'N':
				output_newline = FALSE;
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
				return -1;
		}
	}

	// Process testnet option
	if (output_testnet)
	{
		network_set_test();
	}

	priv = malloc(privkey_sizeof());
	if (priv == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	
	// Process input
	switch (input_format)
	{
		case INPUT_WIF:
			input_len = input_get(&input);

			while (isspace(input[input_len - 1]))
			{
				--input_len;
			}

			input = realloc(input, input_len + 1);
			if (input == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}
			input[input_len] = '\0';

			r = privkey_from_wif(priv, (char *)input);
			if (r < 0)
			{
				error_log("Error while handling input.");
				return -1;
			}

			break;
		case INPUT_HEX:
			input_len = input_get(&input);

			while (isspace((int)input[input_len - 1]))
			{
				--input_len;
			}

			input = realloc(input, input_len + 1);
			if (input == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}
			input[input_len] = '\0';

			r = privkey_from_hex(priv, (char *)input);
			if (r < 0)
			{
				error_log("Error while handling input.");
				return -1;
			}
			break;
		case INPUT_RAW:
			input_len = input_get_from_pipe(&input);
			if (input == NULL)
			{
				error_log("Piped or redirected input required for raw data.");
				return -1;
			}

			r = privkey_from_raw(priv, input, input_len);
			if (r < 0)
			{
				error_log("Error while handling input.");
				return -1;
			}

			break;
		case INPUT_STR:
			input_len = input_get(&input);

			if((int)input[input_len - 1] == '\n')
			{
				--input_len;
			}

			input = realloc(input, input_len + 1);
			if (input == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}
			input[input_len] = '\0';

			r = privkey_from_str(priv, (char *)input);
			if (r < 0)
			{
				error_log("Error while handling input.");
				return -1;
			}

			break;
		case INPUT_DEC:
			input_len = input_get(&input);

			while (isspace((int)input[input_len - 1]))
			{
				--input_len;
			}

			input = realloc(input, input_len + 1);
			if (input == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}
			input[input_len] = '\0';

			r = privkey_from_dec(priv, (char *)input);
			if (r < 0)
			{
				error_log("Error while handling input.");
				return -1;
			}

			break;
		case INPUT_BLOB:
			input_len = input_get_from_pipe(&input);
			if (input == NULL)
			{
				error_log("Piped or redirected input required for blob data.");
				return -1;
			}

			r = privkey_from_blob(priv, input, input_len);
			if (r < 0)
			{
				error_log("Error while handling input.");
				return -1;
			}
			break;
		case INPUT_GUESS:
			input_len = input_get(&input);

			r = privkey_from_guess(priv, input, input_len);
			if (r < 0)
			{
				error_log("Unable to determine input format automatically. Use a command option to specify input format.");
				return -1;
			}

			break;
	}

	// Make sure we have a key
	if (!priv)
	{
		error_log("Unable to generate private key from input.");
		return -1;
	}
	
	// Don't allow the generation of public keys from a zero private key
	if (privkey_is_zero(priv))
	{
		error_log("Invalid private key. Key value cannot be zero.");
		return -1;
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
	key = malloc(pubkey_sizeof());
	if (key == NULL)
	{
		error_log("Memory allocation error");
		return -1;
	}

	r = pubkey_get(key, priv);
	if (r < 0)
	{
		error_log("Error while calculating public key.");
		return -1;
	}

	// Print private key here if flag is set
	if (output_privkey)
	{
		switch  (output_format)
		{
			case OUTPUT_HEX:
				output = malloc(((PRIVKEY_LENGTH + 1) * 2) + 1);
				if (output == NULL)
				{
					error_log("Memory allocation error");
					return -1;
				}
				privkey_to_hex(output, priv);
				printf("%s ", output);
				free(output);
				break;
			case OUTPUT_RAW:
				uc_output = malloc(PRIVKEY_LENGTH + 1);
				if (uc_output == NULL)
				{
					error_log("Memory allocation error");
					return -1;
				}
				output_len = (size_t)privkey_to_raw(uc_output, priv);
				for (i = 0; i < output_len; ++i)
				{
					putchar(uc_output[i]);
				}
				free(uc_output);
				break;
			default:
				output = malloc(PRIVKEY_WIF_LENGTH_MAX + 1);
				if (output == NULL)
				{
					error_log("Memory allocation error");
					return -1;
				}
				privkey_to_wif(output, priv);
				printf("%s ", output);
				free(output);
				break;
		}
	}
	
	// Process output
	switch (output_format)
	{
		case OUTPUT_ADDRESS:
			output = malloc(35);
			if (output == NULL)
			{
				error_log("Memory allocation error");
				return -1;
			}
			r = pubkey_to_address(output, key);
			if (r < 0)
			{
				error_log("Error while calculating address for public key.");
				return -1;
			}
			printf("%s", output);
			free(output);
			break;
		case OUTPUT_BECH32_ADDRESS:
			output = malloc(43);
			if (output == NULL)
			{
				error_log("Memory allocation error");
				return -1;
			}
			r = pubkey_to_bech32address(output, key);
			if (r < 0)
			{
				error_log("Error while calculating bech32 address for public key.");
				return -1;
			}
			printf("%s", output);
			free(output);
			break;
		case OUTPUT_HEX:
			output = malloc(((PUBKEY_UNCOMPRESSED_LENGTH + 1) * 2) + 1);
			if (output == NULL)
			{
				error_log("Memory allocation error");
				return -1;
			}
			r = pubkey_to_hex(output, key);
			if (r < 0)
			{
				error_log("Error while calculating bech32 address for public key.");
				return -1;
			}
			printf("%s", output);
			free(output);
			break;
		case OUTPUT_RAW:
			uc_output = malloc(PUBKEY_UNCOMPRESSED_LENGTH + 1);
			if (uc_output == NULL)
			{
				error_log("Memory allocation error");
				return -1;
			}
			r = pubkey_to_raw(uc_output, key);
			if (r < 0)
			{
				error_log("Error while generating raw data for public key.");
				return -1;
			}
			for (i = 0; i < (size_t)r; ++i)
			{
				putchar(uc_output[i]);
			}
			free(uc_output);
			break;
	}

	switch (output_newline)
	{
		case TRUE:
			printf("\n");
			break;
	}

	free(priv);
	free(key);

	return 1;
}
