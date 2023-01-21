/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include "mods/privkey.h"
#include "mods/network.h"
#include "mods/input.h"
#include "mods/error.h"
#include "mods/json.h"
#include "mods/opts.h"

#define OUTPUT_HASH_MAX    50
#define HASH_WILDCARD      "r"

int btk_privkey_get(PrivKey, unsigned char *, size_t);
int btk_privkey_compression_add(PrivKey);
int btk_privkey_set_network(PrivKey);
int btk_privkey_to_output(char *, PrivKey);
int btk_privkey_process_rehashes(char *);
int btk_privkey_process_rehashes_comp(const void *, const void *);

// Defaults
static int input_type_wif = 0;
static int input_type_hex = 0;
static int input_type_raw = 0;
static int input_type_string = 0;
static int input_type_decimal = 0;
static int input_type_binary = 0;
static int input_type_sbd = 0;
static int output_type_hex = 0;
static int output_type_decimal = 0;
static int compression_on = 0;
static int compression_off = 0;

static int create          = OPTS_CREATE_FALSE;
static int network         = OPTS_OUTPUT_NETWORK_MAINNET;
static char *rehashes      = OPTS_OUTPUT_REHASHES_NONE;

static int output_hashes_arr_len = 0;
static long int output_hashes_arr[OUTPUT_HASH_MAX];

int btk_privkey_main(opts_p opts, unsigned char *input, size_t input_len)
{
	int i, r;
	PrivKey key = NULL;
	long int hash_count;

	assert(opts);

	if (opts->input_type_wif) { input_type_wif =  opts->input_type_wif; }
	if (opts->input_type_hex) { input_type_hex =  opts->input_type_hex; }
	if (opts->input_type_raw) { input_type_raw =  opts->input_type_raw; }
	if (opts->input_type_string) { input_type_string =  opts->input_type_string; }
	if (opts->input_type_decimal) { input_type_decimal =  opts->input_type_decimal; }
	if (opts->input_type_binary) { input_type_binary =  opts->input_type_binary; }
	if (opts->input_type_sbd) { input_type_sbd =  opts->input_type_sbd; }
	if (opts->output_type_hex) { output_type_hex = opts->output_type_hex; }
	if (opts->output_type_decimal) { output_type_decimal = opts->output_type_decimal; }
	if (opts->compression_on) { compression_on = opts->compression_on; }
	if (opts->compression_off) { compression_off = opts->compression_off; }

	if (opts->create) { create = opts->create; }
	if (opts->network) { network = opts->network; }
	if (opts->rehashes) { rehashes = opts->rehashes; }

	key = malloc(privkey_sizeof());
	ERROR_CHECK_NULL(key, "Memory allocation error.");

	if (create)
	{
		r = privkey_new(key);
		ERROR_CHECK_NEG(r, "Could not generate a new private key.");
	}
	else
	{
		ERROR_CHECK_NULL(input, "Input required.");

		r = btk_privkey_get(key, input, input_len);
		ERROR_CHECK_NEG(r, "Could not get privkey from input.");
	}

	r = btk_privkey_set_network(key);
	ERROR_CHECK_NEG(r, "Could not set key network.");

	if (rehashes)
	{
		r = btk_privkey_process_rehashes((char *)input);
		ERROR_CHECK_NEG(r, "Error while processing hash argument [-S].");

		// Perform rehash on key
		for(i = 0; i < output_hashes_arr_len; i++)
		{
			hash_count = output_hashes_arr[i];
			if (i > 0)
			{
				hash_count -= output_hashes_arr[i-1];
			}

			while (hash_count > 0)
			{
				r = privkey_rehash(key);
				ERROR_CHECK_NEG(r, "Unable to rehash private key.");

				hash_count--;
			}

			r = btk_privkey_compression_add(key);
			ERROR_CHECK_NEG(r, "");
		}
	}
	else
	{
		r = btk_privkey_compression_add(key);
		ERROR_CHECK_NEG(r, "");
	}

	free(key);

	return 1;
}

int btk_privkey_get(PrivKey key, unsigned char *input, size_t input_len)
{
	int r;

	assert(key);
	assert(input);

	if (input_type_wif)
	{
		r = privkey_from_wif(key, (char *)input);
	}
	else if (input_type_hex)
	{
		r = privkey_from_hex(key, (char *)input);
	}
	else if (input_type_string)
	{
		r = privkey_from_str(key, (char *)input);
	}
	else if (input_type_decimal)
	{
		r = privkey_from_dec(key, (char *)input);
	}
	else if (input_type_sbd)
	{
		r = privkey_from_sbd(key, (char *)input);
	}
	else if (input_type_raw)
	{
		r = privkey_from_raw(key, input, input_len);
	}
	else if (input_type_binary)
	{
		r = privkey_from_raw(key, input, input_len);
	}
	else
	{
		r = privkey_from_guess(key, input, input_len);
	}
	ERROR_CHECK_NEG(r, "Could not calculate private key from input.");

	if (privkey_is_zero(key))
	{
		error_log("Key value cannot be zero.");
		return -1;
	}

	return 1;
}

int btk_privkey_compression_add(PrivKey key)
{
	int r;
	int comp_on, comp_off;
	char output_str[BUFSIZ];

	comp_on = compression_on;
	comp_off = compression_off;

	comp_again:

	if (comp_on)
	{
		privkey_compress(key);

	}
	else if (comp_off)
	{
		privkey_uncompress(key);

	}

	memset(output_str, 0, BUFSIZ);

	r = btk_privkey_to_output(output_str, key);
	ERROR_CHECK_NEG(r, "Could not get output.");

	r = json_add(output_str);
	ERROR_CHECK_NEG(r, "Error while generating JSON.");

	if (comp_on && comp_off)
	{
		if (privkey_is_compressed(key))
		{
			comp_on = 0;
		}
		else
		{
			comp_off = 0;
		}

		goto comp_again;
	}

	return 1;
}

int btk_privkey_set_network(PrivKey key)
{
	assert(key);

	switch (network)
	{
		case OPTS_OUTPUT_NETWORK_NONE:
			break;
		case OPTS_OUTPUT_NETWORK_MAINNET:
			network_set_main();
			break;
		case OPTS_OUTPUT_NETWORK_TESTNET:
			network_set_test();
			break;
	}

	return 1;
}

int btk_privkey_to_output(char *output, PrivKey key)
{
	int r;

	assert(output);
	assert(key);

	if (output_type_hex)
	{
		r = privkey_to_hex(output, key, (compression_on || compression_off) ? 0 : 1);
		ERROR_CHECK_NEG(r, "Could not convert private key to hex format.");
	}
	else if (output_type_decimal)
	{
		r = privkey_to_dec(output, key);
		ERROR_CHECK_NEG(r, "Could not convert private key to decimal format.");
	}
	else
	{
		r = privkey_to_wif(output, key);
		ERROR_CHECK_NEG(r, "Could not convert private key to WIF format.");
	}

	return 1;
}

int btk_privkey_process_rehashes(char *input_str)
{
	int i, j;
	int output_hashes_len = 0;
	char *tok;
	char *tokend;
	long int tmp;

	// Save a pointer to the start of output_hashes so that it can be reset
	// for list processing.
	output_hashes_len = strlen(rehashes);

	i = 0;
	tok = strtok(rehashes, ",");
	while (tok != NULL)
	{
		tmp = strtol(tok, &tokend, 10);
		if (strcmp(tokend, "") == 0)
		{
			if (tmp < 0)
			{
				error_log("Argument cannot contain a negative number: %ld", tmp);
				return -1;
			}

			output_hashes_arr[i++] = tmp;
		}
		else if (strcmp(tok, HASH_WILDCARD) == 0)
		{
			if (!input_type_string && !input_type_decimal)
			{
				error_log("Can not use wildcard '%s' with current input mode.", HASH_WILDCARD);
				return -1;
			}

			tokend = input_str;
			while (*tokend != '\0')
			{
				tmp = strtol(input_str, &tokend, 10);
				if (tmp > 0)
				{
					output_hashes_arr[i++] = tmp;
				}
				else if (tmp < 0)
				{
					output_hashes_arr[i++] = labs(tmp);
				}
				if (*tokend != '\0')
				{
					input_str = tokend + 1;
				}
			}

		}
		else
		{
			error_log("Invalid rehash argument: '%s'. Must be numbers only.", tok);
			return -1;
		}

		tok = strtok(NULL, ",");
	}

	qsort(output_hashes_arr, i, sizeof(long int), btk_privkey_process_rehashes_comp);

	output_hashes_arr_len = i;

	// Get rid of dups
	for (i = 0; i < output_hashes_arr_len-1; i++)
	{
		if (output_hashes_arr[i] == output_hashes_arr[i+1])
		{
			for (j = i; j < output_hashes_arr_len-1; j++)
			{
				output_hashes_arr[j] = output_hashes_arr[j+1];
			}
			output_hashes_arr_len--;
			i--;
		}
	}

	// Undo the strtok changes to output_hashes for use in next item in input
	// list (if exists).
	for (i = 0; i < output_hashes_len; i++)
	{
		if (rehashes[i] == '\0')
		{
			rehashes[i] = ',';
		}
	}

	return 1;
}

int btk_privkey_process_rehashes_comp(const void *i, const void *j)
{
	return (*(long int *)i - *(long int *)j);
}

int btk_privkey_requires_input(opts_p opts)
{
	assert(opts);

	if (opts->create)
	{
		return 0;
	}

	return 1;
}