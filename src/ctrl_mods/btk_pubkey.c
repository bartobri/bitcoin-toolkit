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
#include "mods/mem.h"
#include "mods/assert.h"

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
				return EXIT_FAILURE;
		}
	}

	// Process testnet option
	if (output_testnet)
	{
		network_set_test();
	}
	
	// Process input
	switch (input_format)
	{
		case INPUT_WIF:
			input_len = input_get(&input);

			while (isspace((int)input[input_len - 1]))
			{
				--input_len;
			}

			if (input_len < PRIVKEY_WIF_LENGTH_MIN)
			{
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}

			for (i = 0; i < input_len; ++i)
			{
				if (!base58_ischar(input[i]))
				{
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}

			if (!base58check_valid_checksum((char *)input, input_len))
			{
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}

			RESIZE(input, input_len + 1);
			input[input_len] = '\0';

			priv = ALLOC(privkey_sizeof());
			r = privkey_from_wif(priv, (char *)input);
			if (r < 0)
			{
				fprintf(stderr, "Error: Invalid WIF string.\n");
				return EXIT_FAILURE;
			}
			break;
		case INPUT_HEX:
			input_len = input_get(&input);

			while (isspace((int)input[input_len - 1]))
			{
				--input_len;
			}

			if (input_len != PRIVKEY_LENGTH * 2 && input_len != (PRIVKEY_LENGTH + 1) * 2)
			{
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}

			for (i = 0; i < input_len; ++i)
			{
				if (!hex_ischar(input[i]))
				{
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}

			RESIZE(input, input_len + 1);
			input[input_len] = '\0';

			priv = ALLOC(privkey_sizeof());
			r = privkey_from_hex(priv, (char *)input);
			if (r < 0)
			{
				fprintf(stderr, "Error: Invalid hex string.\n");
				return EXIT_FAILURE;
			}
			break;
		case INPUT_RAW:
			input_len = input_get_from_pipe(&input);
			if (input == NULL)
			{
				fprintf(stderr, "Error: Input required.\n");
				return EXIT_FAILURE;
			}

			if (input_len < PRIVKEY_LENGTH)
			{
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}

			priv = ALLOC(privkey_sizeof());
			r = privkey_from_raw(priv, input, input_len);
			if (r < 0)
			{
				fprintf(stderr, "Error: Invalid raw data.\n");
				return EXIT_FAILURE;
			}

			break;
		case INPUT_STR:
			input_len = input_get(&input);

			if((int)input[input_len - 1] == '\n')
			{
				--input_len;
			}

			RESIZE(input, input_len + 1);
			input[input_len] = '\0';

			priv = ALLOC(privkey_sizeof());
			r = privkey_from_str(priv, (char *)input);
			if (r < 0)
			{
				fprintf(stderr, "Error: Invalid string.\n");
				return EXIT_FAILURE;
			}

			break;
		case INPUT_DEC:
			input_len = input_get(&input);

			while (isspace((int)input[input_len - 1]))
			{
				--input_len;
			}

			for (i = 0; i < input_len; ++i)
			{
				if (input[i] < '0' || input[i] > '9')
				{
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}

			RESIZE(input, input_len + 1);
			input[input_len] = '\0';

			priv = ALLOC(privkey_sizeof());
			r = privkey_from_dec(priv, (char *)input);
			if (r < 0)
			{
				fprintf(stderr, "Error: Invalid decimal string.\n");
				return EXIT_FAILURE;
			}

			break;
		case INPUT_BLOB:
			input_len = input_get_from_pipe(&input);
			if (input == NULL)
			{
				fprintf(stderr, "Error: Input required.\n");
				return EXIT_FAILURE;
			}

			priv = ALLOC(privkey_sizeof());
			r = privkey_from_blob(priv, input, input_len);
			if (r < 0)
			{
				fprintf(stderr, "Error: Invalid blob.\n");
				return EXIT_FAILURE;
			}
			break;
		case INPUT_GUESS:
			input_len = input_get(&input);

			priv = privkey_from_guess(input, input_len);

			if (priv == NULL)
			{
				fprintf(stderr, "Error: Unable to interpret input automatically. Use an input switch to specify how this input should be interpreted.\n");
				return EXIT_FAILURE;
			}

			break;
	}

	assert(priv);
	
	// Don't allow the generation of public keys from a zero private key
	if (privkey_is_zero(priv))
	{
		fprintf(stderr, "Error: Private key can not be zero.\n");
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
	key = pubkey_get(priv);

	// Ensure we have a key
	if (!key)
	{
		fprintf(stderr, "Unable to generate public key. Input required.\n");
		return EXIT_FAILURE;
	}

	// Print private key here if flag is set
	if (output_privkey)
	{
		switch  (output_format)
		{
			case OUTPUT_HEX:
				output = ALLOC(((PRIVKEY_LENGTH + 1) * 2) + 1);
				privkey_to_hex(output, priv);
				printf("%s ", output);
				FREE(output);
				break;
			case OUTPUT_RAW:
				uc_output = ALLOC(PRIVKEY_LENGTH + 1);
				output_len = (size_t)privkey_to_raw(uc_output, priv);
				for (i = 0; i < output_len; ++i)
				{
					putchar(uc_output[i]);
				}
				FREE(uc_output);
				break;
			default:
				output = ALLOC(PRIVKEY_WIF_LENGTH_MAX + 1);
				privkey_to_wif(output, priv);
				printf("%s ", output);
				FREE(output);
				break;
		}
	}
	
	// Process output
	switch (output_format)
	{
		case OUTPUT_ADDRESS:
			printf("%s", pubkey_to_address(key));
			break;
		case OUTPUT_BECH32_ADDRESS:
			if(!pubkey_is_compressed(key))
			{
				fprintf(stderr, "Error: Can not use an uncompressed private key for a bech32 address.\n");
				return EXIT_FAILURE;
			}
			printf("%s", pubkey_to_bech32address(key));
			break;
		case OUTPUT_HEX:
			printf("%s", pubkey_to_hex(key));
			break;
		case OUTPUT_RAW:
			uc_output = pubkey_to_raw(key, &output_len);
			for (i = 0; i < output_len; ++i)
			{
				putchar(uc_output[i]);
			}
			FREE(uc_output);
			break;
	}

	// Process newline flag
	switch (output_newline)
	{
		case TRUE:
			printf("\n");
			break;
	}

	// Free allocated memory
	privkey_free(priv);
	pubkey_free(key);

	return EXIT_SUCCESS;
}
