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

#define OUTPUT_BUFFER           150

int btk_pubkey_main(int argc, char *argv[])
{
	int o, r;
	PubKey key = NULL;
	PrivKey priv = NULL;
	size_t i;
	unsigned char *input_uc;
	char *input_sc;
	size_t output_len;
	char output[OUTPUT_BUFFER];
	unsigned char uc_output[OUTPUT_BUFFER];

	int input_format       = INPUT_GUESS;
	int output_format      = OUTPUT_ADDRESS;
	int output_compression = FALSE;
	int output_privkey     = FALSE;
	int output_newline     = TRUE;
	int output_testnet     = FALSE;
	
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
					error_log("Unknown option '-%c'.", optopt);
				}
				else
				{
					error_log("Unknown option character '\\x%x'.", optopt);
				}
				return -1;
		}
	}

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
	
	switch (input_format)
	{
		case INPUT_WIF:
			r = input_get_str(&input_sc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = privkey_from_wif(priv, input_sc);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_sc);
			break;
		case INPUT_HEX:
			r = input_get_str(&input_sc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = privkey_from_hex(priv, input_sc);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_sc);
			break;
		case INPUT_RAW:
			r = input_get_from_pipe(&input_uc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}
			if (r == 0)
			{
				error_log("Piped or redirected input required for raw data.");
				return -1;
			}

			r = privkey_from_raw(priv, input_uc, r);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_uc);
			break;
		case INPUT_STR:
			r = input_get_str(&input_sc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = privkey_from_str(priv, input_sc);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_sc);
			break;
		case INPUT_DEC:
			r = input_get_str(&input_sc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = privkey_from_dec(priv, input_sc);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_sc);
			break;
		case INPUT_BLOB:
			r = input_get_from_pipe(&input_uc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}
			if (r == 0)
			{
				error_log("Piped or redirected input required for blob data.");
				return -1;
			}

			r = privkey_from_blob(priv, input_uc, r);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_uc);
			break;
		case INPUT_GUESS:
			r = input_get(&input_uc);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = privkey_from_guess(priv, input_uc, r);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}

			free(input_uc);
			break;
	}

	if (!priv)
	{
		error_log("Could not calculate private key from input.");
		return -1;
	}
	
	if (privkey_is_zero(priv))
	{
		error_log("Key value cannot be zero.");
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
		error_log("Could not calculate public key.");
		return -1;
	}

	memset(output, 0, OUTPUT_BUFFER);
	memset(uc_output, 0, OUTPUT_BUFFER);

	if (output_privkey)
	{
		switch  (output_format)
		{
			case OUTPUT_HEX:
				r = privkey_to_hex(output, priv);
				if (r < 0)
				{
					error_log("Could not convert private key to hex format.");
					return -1;
				}
				printf("%s ", output);
				break;
			case OUTPUT_RAW:
				r = privkey_to_raw(uc_output, priv);
				if (r < 0)
				{
					error_log("Could not convert private key to raw format.");
					return -1;
				}
				output_len = (size_t)r;
				for (i = 0; i < output_len; ++i)
				{
					putchar(uc_output[i]);
				}
				break;
			default:
				r = privkey_to_wif(output, priv);
				if (r < 0)
				{
					error_log("Could not convert private key to WIF format.");
					return -1;
				}
				printf("%s ", output);
				break;
		}
	}

	memset(output, 0, OUTPUT_BUFFER);
	memset(uc_output, 0, OUTPUT_BUFFER);

	switch (output_format)
	{
		case OUTPUT_ADDRESS:
			r = pubkey_to_address(output, key);
			if (r < 0)
			{
				error_log("Could not calculate public key address.");
				return -1;
			}
			printf("%s", output);
			break;
		case OUTPUT_BECH32_ADDRESS:
			r = pubkey_to_bech32address(output, key);
			if (r < 0)
			{
				error_log("Could not calculate bech32 public key address.");
				return -1;
			}
			printf("%s", output);
			break;
		case OUTPUT_HEX:
			r = pubkey_to_hex(output, key);
			if (r < 0)
			{
				error_log("Could not generate hex data from public key.");
				return -1;
			}
			printf("%s", output);
			break;
		case OUTPUT_RAW:
			r = pubkey_to_raw(uc_output, key);
			if (r < 0)
			{
				error_log("Could not generate raw data for public key.");
				return -1;
			}
			for (i = 0; i < (size_t)r; ++i)
			{
				putchar(uc_output[i]);
			}
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
