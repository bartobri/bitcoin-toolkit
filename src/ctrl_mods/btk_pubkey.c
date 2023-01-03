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
#include <assert.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/input.h"
#include "mods/json.h"
#include "mods/opts.h"
#include "mods/error.h"

int btk_pubkey_set_compression(PubKey);

static opts_p opts;

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

	r = opts_get(&opts);
	ERROR_CHECK_NEG(r, "Could not get command options.");

	r = input_get(&input, &input_len);
	ERROR_CHECK_NEG(r, "Error getting input.");

	if (opts->input_format == OPTS_INPUT_FORMAT_ASCII)
	{
		r = json_from_input(&input, &input_len);
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

			switch (opts->input_type)
			{
				case OPTS_INPUT_TYPE_WIF:
					r = privkey_from_wif(privkey, input_str);
					ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
					r = pubkey_get(pubkey, privkey);
					ERROR_CHECK_NEG(r, "Could not calculate public key.");
					break;
				case OPTS_INPUT_TYPE_HEX:
					r = pubkey_from_hex(pubkey, input_str);
					ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
					break;
				case OPTS_INPUT_TYPE_GUESS:
					r = pubkey_from_guess(pubkey, (unsigned char *)input_str, strlen(input_str));
					ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
					break;
			}

			if (opts->input_type == OPTS_INPUT_TYPE_WIF && opts->compression == OPTS_OUTPUT_COMPRESSION_NONE)
			{
				if (privkey_is_compressed(privkey))
				{
					pubkey_compress(pubkey);
				}
				else
				{
					pubkey_uncompress(pubkey);
				}
			}
			else
			{
				r = btk_pubkey_set_compression(pubkey);
				ERROR_CHECK_NEG(r, "Could not set compression for public key.");
			}

			r = pubkey_to_hex(output_str, pubkey);
			ERROR_CHECK_NEG(r, "Could not get output.");

			r = json_add(output_str);
			ERROR_CHECK_NEG(r, "Error while generating JSON.");

			if (opts->compression == OPTS_OUTPUT_COMPRESSION_BOTH)
			{
				memset(output_str, 0, BUFSIZ);
				
				r = btk_pubkey_set_compression(pubkey);
				ERROR_CHECK_NEG(r, "Could not set compression for public key.");

				r = pubkey_to_hex(output_str, pubkey);
				ERROR_CHECK_NEG(r, "Could not get output.");

				r = json_add(output_str);
				ERROR_CHECK_NEG(r, "Error while generating JSON.");
			}
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

int btk_pubkey_set_compression(PubKey key)
{
	static int last = 0;

	assert(key);

	switch (opts->compression)
	{
		case OPTS_OUTPUT_COMPRESSION_NONE:
		case OPTS_OUTPUT_COMPRESSION_TRUE:
			pubkey_compress(key);
			break;
		case OPTS_OUTPUT_COMPRESSION_FALSE:
			pubkey_uncompress(key);
			break;
		case OPTS_OUTPUT_COMPRESSION_BOTH:
			if (last == OPTS_OUTPUT_COMPRESSION_TRUE)
			{
				pubkey_uncompress(key);
				last = OPTS_OUTPUT_COMPRESSION_FALSE;
			}
			else
			{
				pubkey_compress(key);
				last = OPTS_OUTPUT_COMPRESSION_TRUE;
			}
			break;
	}

	return 1;
}