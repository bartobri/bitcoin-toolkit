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
#include "mods/json.h"
#include "mods/error.h"

#define INPUT_ASCII             1
#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_GUESS             4
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

#define INPUT_SET_FORMAT(x)     if (input_format == FALSE) { input_format = x; } else { error_log("Cannot specify ascii input format."); return -1; }
#define INPUT_SET(x)            if (input_type == FALSE) { input_type = x; } else { error_log("Cannot use multiple input format flags."); return -1; }
#define COMPRESSION_SET(x)      if (output_compression == FALSE) { output_compression = x; } else { error_log("Only specify one compression flag."); return -1; }

int btk_pubkey_input_to_json(unsigned char **, size_t *);

static int input_format         = FALSE;
static int input_type           = FALSE;
static int output_compression   = FALSE;

int btk_pubkey_init(int argc, char *argv[])
{
	int o;
	char *command = NULL;

	command = argv[1];

	while ((o = getopt(argc, argv, "whaCU")) != -1)
	{
		switch (o)
		{
			// Input format
			case 'a':
				INPUT_SET_FORMAT(INPUT_ASCII);
				break;

			// Input type
			case 'w':
				INPUT_SET(INPUT_WIF);
				break;
			case 'h':
				INPUT_SET(INPUT_HEX);
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

	if (input_type == FALSE)
	{
		input_type = INPUT_GUESS;
	}

	return 1;
}

int btk_pubkey_main(void)
{
	int r;
	size_t i, len, input_len;
	unsigned char *input; 
	char input_str[BUFSIZ];
	char output_str[BUFSIZ];
	PubKey pubkey = NULL;
	PrivKey privkey = NULL;

	memset(input_str, 0, BUFSIZ);
	memset(output_str, 0, BUFSIZ);

	privkey = malloc(privkey_sizeof());
	ERROR_CHECK_NULL(privkey, "Memory allocation error.");

	pubkey = malloc(pubkey_sizeof());
	ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

	json_init();

	r = input_get(&input, &input_len);
	ERROR_CHECK_NEG(r, "Error getting input.");

	if (input_format == INPUT_ASCII)
	{
		r = btk_pubkey_input_to_json(&input, &input_len);
		ERROR_CHECK_NEG(r, "Could not convert input to JSON.");
	}

	if(json_is_valid((char *)input, input_len))
	{
		r = json_set_input((char *)input);
		ERROR_CHECK_NEG(r, "Could not load JSON input.");

		r = json_get_input_len((int *)&len);
		ERROR_CHECK_NEG(r, "Could not get input list length.");

		for (i = 0; i < len; i++)
		{
			memset(input_str, 0, BUFSIZ);

			r = json_get_input_index(input_str, BUFSIZ, i);
			ERROR_CHECK_NEG(r, "Could not get JSON string object at index.");

			switch (input_type)
			{
				case INPUT_WIF:
					r = privkey_from_wif(privkey, input_str);
					ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
					r = pubkey_get(pubkey, privkey);
					ERROR_CHECK_NEG(r, "Could not calculate public key.");
					break;
				case INPUT_HEX:
					r = pubkey_from_hex(pubkey, input_str);
					ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
					break;
				case INPUT_GUESS:
					r = pubkey_from_guess(pubkey, (unsigned char *)input_str, strlen(input_str));
					ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
					break;
			}

			switch (output_compression)
			{
				case FALSE:
					break;
				case OUTPUT_COMPRESS:
					pubkey_compress(pubkey);
					break;
				case OUTPUT_UNCOMPRESS:
					pubkey_uncompress(pubkey);
					break;
			}

			r = pubkey_to_hex(output_str, pubkey);
			ERROR_CHECK_NEG(r, "Could not get output.");

			r = json_add(output_str);
			ERROR_CHECK_NEG(r, "Error while generating JSON.");
		}
	}
	else
	{
		error_log("Invalid JSON. Input must be in JSON format or specify a non-JSON input format.");
		return -1;
	}

	json_print();
	json_free();
	
	free(pubkey);
	free(privkey);

	return 1;
}

int btk_pubkey_cleanup(void)
{
	return 1;
}

int btk_pubkey_input_to_json(unsigned char **input, size_t *input_len)
{
	int r;
	size_t i;
	char str[BUFSIZ];

	memset(str, 0, BUFSIZ);

	for (i = 0; i < *input_len; i++)
	{
		if (i > BUFSIZ-1)
		{
			error_log("Input string too large. Consider using -b for arbitrarily large amounts of input data.");
			return -1;
		}
		if (isascii((*input)[i]))
		{
			str[i] = (*input)[i];
		}
		else
		{
			error_log("Input contains non-ascii characters.");
			return -1;
		}
	}

	while (str[(*input_len)-1] == '\n' || str[(*input_len)-1] == '\r')
	{
		str[(*input_len)-1] = '\0';
		(*input_len)--;
	}

	// Free input here because json_from_string reallocates it.
	free(*input);

	r = json_from_string((char **)input, str);
	ERROR_CHECK_NEG(r, "Could not convert input to JSON.");

	*input_len = strlen((char *)*input);

	return 1;
}