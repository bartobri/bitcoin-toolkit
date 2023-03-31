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
#include "mods/output.h"
#include "mods/opts.h"
#include "mods/error.h"

static int compression_on = 0;
static int compression_off = 0;

int btk_pubkey_main(output_item *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int r;
	char input_str[BUFSIZ];
	char output_str[BUFSIZ];
	PubKey pubkey = NULL;
	PrivKey privkey = NULL;

	assert(opts);

	if (opts->compression_on) { compression_on = opts->compression_on; }
	if (opts->compression_off) { compression_off = opts->compression_off; }

	memset(input_str, 0, BUFSIZ);
	memset(output_str, 0, BUFSIZ);

	privkey = malloc(privkey_sizeof());
	ERROR_CHECK_NULL(privkey, "Memory allocation error.");

	pubkey = malloc(pubkey_sizeof());
	ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

	if (opts->input_type_wif)
	{
		memcpy(input_str, input, input_len);

		r = privkey_from_wif(privkey, input_str);
		ERROR_CHECK_NEG(r, "Could not calculate private key from input.");

		r = pubkey_get(pubkey, privkey);
		ERROR_CHECK_NEG(r, "Could not calculate public key.");
	}
	else if (opts->input_type_hex)
	{
		memcpy(input_str, input, input_len);

		r = pubkey_from_hex(pubkey, input_str);
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

	if (opts->input_type_wif)
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

	compression_again:

	if (compression_on)
	{
		pubkey_compress(pubkey);

	}
	else if (compression_off)
	{
		pubkey_uncompress(pubkey);
	}
	
	r = pubkey_to_hex(output_str, pubkey);
	ERROR_CHECK_NEG(r, "Could not get output.");

	*output = output_append_new_copy(*output, output_str, strlen(output_str) + 1);
	ERROR_CHECK_NULL(*output, "Memory allocation error.");

	if (compression_on && compression_off)
	{
		if (pubkey_is_compressed(pubkey))
		{
			compression_on = 0;
		}
		else
		{
			compression_off = 0;
		}

		goto compression_again;
	}
	
	free(pubkey);
	free(privkey);

	return 1;
}

int btk_pubkey_requires_input(opts_p opts)
{
	assert(opts);

	return 1;
}

int btk_pubkey_init(opts_p opts)
{
	assert(opts);

	if (opts->output_type_wif || opts->output_type_decimal || opts->output_type_raw)
	{
		error_log("Invalid output type option.");
		return -1;
	}

	return 1;
}

int btk_pubkey_cleanup(opts_p opts)
{
	assert(opts);
	
	return 1;
}