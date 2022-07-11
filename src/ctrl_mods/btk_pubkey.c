/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/input.h"
#include "mods/error.h"

#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_RAW               3
#define INPUT_GUESS             4
#define OUTPUT_HEX              1
#define OUTPUT_RAW              2
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0
#define OUTPUT_BUFFER           150

#define INPUT_SET(x)            if (input_format == FALSE) { input_format = x; } else { error_log("Cannot use multiple input format flags."); return -1; }
#define OUTPUT_SET(x)           if (output_format == FALSE) { output_format = x; } else { error_log("Cannot use multiple output format flags."); return -1; }
#define COMPRESSION_SET(x)      if (output_compression == FALSE) { output_compression = x; } else { error_log("Only specify one compression flag."); return -1; }

static int input_format         = FALSE;
static int output_format        = FALSE;
static int output_compression   = FALSE;

int btk_pubkey_init(int argc, char *argv[])
{
	int o;
	char *command = NULL;

	command = argv[1];

	while ((o = getopt(argc, argv, "whrHRCU")) != -1)
	{
		switch (o)
		{
			// Input format
			case 'w':
				INPUT_SET(INPUT_WIF)
				break;
			case 'h':
				INPUT_SET(INPUT_HEX);
				break;
			case 'r':
				INPUT_SET(INPUT_RAW);
				break;

			// Output format
			case 'H':
				OUTPUT_SET(OUTPUT_HEX);
				break;
			case 'R':
				OUTPUT_SET(OUTPUT_RAW);
				break;

				// Output Compression
			case 'C':
				COMPRESSION_SET(OUTPUT_COMPRESS);
				break;
			case 'U':
				COMPRESSION_SET(OUTPUT_UNCOMPRESS);
				break;

			// Unknown option
			case '?':
				error_log("See 'btk help %s' to read about available argument options.", command);
				if (isprint(optopt))
				{
					error_log("Invalid command option or argument required: '-%c'.", optopt);
				}
				else
				{
					error_log("Invalid command option character '\\x%x'.", optopt);
				}
				return -1;
		}
	}

	if (input_format == FALSE)
	{
		input_format = INPUT_GUESS;
	}

	if (output_format == FALSE)
	{
		output_format = OUTPUT_HEX;
	}

	return 1;
}

int btk_pubkey_main(void)
{
	int r;
	PubKey key = NULL;
	PrivKey priv = NULL;
	size_t i;
	unsigned char *input_uc;
	char *input_sc;
	char output[OUTPUT_BUFFER];
	unsigned char uc_output[OUTPUT_BUFFER];

	key = malloc(pubkey_sizeof());
	if (key == NULL)
	{
		error_log("Memory allocation error");
		return -1;
	}
	
	switch (input_format)
	{
		case INPUT_WIF:
			priv = malloc(privkey_sizeof());
			if (priv == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			r = input_get_str(&input_sc, NULL);
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

			if (privkey_is_zero(priv))
			{
				error_log("Private key decimal value cannot be zero.");
				return -1;
			}

			r = pubkey_get(key, priv);
			if (r < 0)
			{
				error_log("Could not calculate public key.");
				return -1;
			}

			free(input_sc);
			free(priv);
			break;
		case INPUT_HEX:
			r = input_get_str(&input_sc, NULL);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = pubkey_from_hex(key, input_sc);
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

			r = pubkey_from_raw(key, input_uc, r);
			if (r < 0)
			{
				error_log("Could not get public key from input.");
				return -1;
			}

			free(input_uc);
			break;
		case INPUT_GUESS:
			r = input_get(&input_uc, NULL, INPUT_GET_MODE_ALL);
			if (r < 0)
			{
				error_log("Could not get input.");
				return -1;
			}

			r = pubkey_from_guess(key, input_uc, r);
			if (r < 0)
			{
				error_log("Could not get public key from input.");
				return -1;
			}

			free(input_uc);
			break;
	}

	if (!key)
	{
		error_log("Could not get public key from input.");
		return -1;
	}

	switch (output_compression)
	{
		case FALSE:
			break;
		case OUTPUT_COMPRESS:
			pubkey_compress(key);
			break;
		case OUTPUT_UNCOMPRESS:
			pubkey_decompress(key);
			break;
	}

	memset(output, 0, OUTPUT_BUFFER);
	memset(uc_output, 0, OUTPUT_BUFFER);

	switch (output_format)
	{
		case OUTPUT_HEX:
			r = pubkey_to_hex(output, key);
			if (r < 0)
			{
				error_log("Could not generate hex data from public key.");
				return -1;
			}
			printf("%s\n", output);
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

	free(key);

	return 1;
}

int btk_pubkey_cleanup(void)
{
	return 1;
}
