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

// Defaults
static int compression     = OPTS_OUTPUT_COMPRESSION_NONE;

int btk_pubkey_main(opts_p opts, unsigned char *input, size_t input_len)
{
	int r;
	char output_str[BUFSIZ];
	PubKey pubkey = NULL;
	PrivKey privkey = NULL;

	assert(opts);

	if (opts->compression) { compression = opts->compression; }

	memset(output_str, 0, BUFSIZ);

	privkey = malloc(privkey_sizeof());
	ERROR_CHECK_NULL(privkey, "Memory allocation error.");

	pubkey = malloc(pubkey_sizeof());
	ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

	if (opts->input_type_wif)
	{
		r = privkey_from_wif(privkey, (char *)input);
		ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
		r = pubkey_get(pubkey, privkey);
		ERROR_CHECK_NEG(r, "Could not calculate public key.");
	}
	else if (opts->input_type_hex)
	{
		r = pubkey_from_hex(pubkey, (char *)input);
		ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
	}
	else
	{
		r = pubkey_from_guess(pubkey, input, input_len);
		if (r < 0)
		{
			error_clear();
			ERROR_CHECK_NEG(-1, "Invalid or missing input type specified.");
		}
	}

	if (opts->input_type_wif && compression == OPTS_OUTPUT_COMPRESSION_NONE)
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

	if (compression == OPTS_OUTPUT_COMPRESSION_BOTH)
	{
		memset(output_str, 0, BUFSIZ);
		
		r = btk_pubkey_set_compression(pubkey);
		ERROR_CHECK_NEG(r, "Could not set compression for public key.");

		r = pubkey_to_hex(output_str, pubkey);
		ERROR_CHECK_NEG(r, "Could not get output.");

		r = json_add(output_str);
		ERROR_CHECK_NEG(r, "Error while generating JSON.");
	}
	
	free(pubkey);
	free(privkey);

	return 1;
}

int btk_pubkey_set_compression(PubKey key)
{
	static int last = 0;

	assert(key);

	switch (compression)
	{
		case OPTS_OUTPUT_COMPRESSION_NONE:
			break;
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

int btk_pubkey_requires_input(opts_p opts)
{
	assert(opts);

	return 1;
}