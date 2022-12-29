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

int btk_privkey_get(PrivKey, char *, unsigned char *, size_t);
int btk_privkey_hashes_args_add(PrivKey, char *);
int btk_privkey_args_add(PrivKey);
int btk_privkey_set_compression(PrivKey);
int btk_privkey_set_network(PrivKey);
int btk_privkey_to_output(char *, PrivKey);
int btk_privkey_output_hashes_process(char *);
int btk_privkey_output_hashes_comp(const void *, const void *);
int btk_privkey_rehash(PrivKey, int);

static opts_p opts;
static long int output_hashes_arr[OUTPUT_HASH_MAX];
static int output_hashes_arr_len = 0;

int btk_privkey_init(void)
{
	int r;

	r = opts_get(&opts);
	ERROR_CHECK_NEG(r, "Could not get command options.");

	return 1;
}

int btk_privkey_main(void)
{
	size_t i, len;
	int r;
	PrivKey key = NULL;
	unsigned char *input; 
	size_t input_len;
	char input_str[BUFSIZ];

	memset(input_str, 0, BUFSIZ);

	key = malloc(privkey_sizeof());
	ERROR_CHECK_NULL(key, "Memory allocation error.");

	json_init();

	if (opts->create)
	{
		r = privkey_new(key);
		ERROR_CHECK_NEG(r, "Could not generate a new private key.");

		if (opts->rehashes)
		{
			r = btk_privkey_hashes_args_add(key, input_str);
			ERROR_CHECK_NEG(r, "");
		}
		else
		{
			r = btk_privkey_args_add(key);
			ERROR_CHECK_NEG(r, "");
		}
	}
	else
	{
		r = input_get(&input, &input_len);
		ERROR_CHECK_NEG(r, "Error getting input.");

		if (opts->input_format == OPTS_INPUT_FORMAT_ASCII)
		{
			r = json_from_input(&input, &input_len);
			ERROR_CHECK_NEG(r, "Could not convert input to JSON.");

			opts->input_format = OPTS_INPUT_FORMAT_JSON;
		}

		if (opts->input_format == OPTS_INPUT_FORMAT_JSON)
		{
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

					r = btk_privkey_get(key, input_str, NULL, 0);
					ERROR_CHECK_NEG(r, "Could not get privkey from input.");

					if (opts->rehashes)
					{
						r = btk_privkey_hashes_args_add(key, input_str);
						ERROR_CHECK_NEG(r, "");
					}
					else
					{
						r = btk_privkey_args_add(key);
						ERROR_CHECK_NEG(r, "");
					}
				}
			}
			else
			{
				error_log("Invalid JSON. Input must be in JSON format or specify a non-JSON input format.");
				return -1;
			}
		}
		else if (opts->input_format == OPTS_INPUT_FORMAT_BINARY)
		{
			r = btk_privkey_get(key, NULL, input, input_len);
			ERROR_CHECK_NEG(r, "Could not get privkey from input.");

			if (opts->rehashes)
			{
				r = btk_privkey_hashes_args_add(key, (char *)NULL);
				ERROR_CHECK_NEG(r, "");
			}
			else
			{
				r = btk_privkey_args_add(key);
				ERROR_CHECK_NEG(r, "");
			}
		}

		free(input);
	}

	json_print();
	json_free();

	free(key);

	return 1;
}

int btk_privkey_cleanup(void)
{
	return 1;
}

int btk_privkey_get(PrivKey key, char *sc_input, unsigned char *uc_input, size_t uc_input_len)
{
	int r;

	assert(key);
	assert(sc_input || uc_input);

	if (opts->create)
	{
		r = privkey_new(key);
		ERROR_CHECK_NEG(r, "Could not generate a new private key.");
		
		return 1;
	}

	switch (opts->input_type)
	{		
		case OPTS_INPUT_TYPE_WIF:
			r = privkey_from_wif(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_HEX:
			r = privkey_from_hex(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_STRING:
			r = privkey_from_str(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_DECIMAL:
			r = privkey_from_dec(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_SBD:
			r = privkey_from_sbd(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_RAW:
			r = privkey_from_raw(key, uc_input, uc_input_len);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_BINARY:
			r = privkey_from_blob(key, uc_input, uc_input_len);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case OPTS_INPUT_TYPE_GUESS:
			if (uc_input)
			{
				r = privkey_from_guess(key, uc_input, uc_input_len);
				ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			}
			else
			{
				r = privkey_from_guess(key, (unsigned char *)sc_input, strlen(sc_input));
				ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			}
			break;
	}

	if (privkey_is_zero(key))
	{
		error_log("Key value cannot be zero.");
		return -1;
	}

	return 1;
}

int btk_privkey_hashes_args_add(PrivKey key, char *input_str)
{
	int r, i;

	r = btk_privkey_output_hashes_process(input_str);
	ERROR_CHECK_NEG(r, "Error while processing hash argument [-S].");

	if (output_hashes_arr_len > 0)
	{
		for(i = 0; i < output_hashes_arr_len; i++)
		{
			r = btk_privkey_rehash(key, i);
			ERROR_CHECK_NEG(r, "Unable to rehash private key.");

			r = btk_privkey_args_add(key);
			ERROR_CHECK_NEG(r, "");
		}
	}
	else
	{
		r = btk_privkey_args_add(key);
		ERROR_CHECK_NEG(r, "");
	}

	return 1;
}

int btk_privkey_args_add(PrivKey key)
{
	int r;
	char output_str[BUFSIZ];

	// In theory, no output type should ever overrun this buffer (famous last words).
	memset(output_str, 0, BUFSIZ);

	r = btk_privkey_set_network(key);
	ERROR_CHECK_NEG(r, "Could not set key network.");

	r = btk_privkey_set_compression(key);
	ERROR_CHECK_NEG(r, "Could not set key compression.");

	r = btk_privkey_to_output(output_str, key);
	ERROR_CHECK_NEG(r, "Could not get output.");

	r = json_add(output_str);
	ERROR_CHECK_NEG(r, "Error while generating JSON.");

	if (opts->compression == OPTS_OUTPUT_COMPRESSION_BOTH)
	{
		memset(output_str, 0, BUFSIZ);

		r = btk_privkey_set_compression(key);
		ERROR_CHECK_NEG(r, "Could not set key compression.");

		r = btk_privkey_to_output(output_str, key);
		ERROR_CHECK_NEG(r, "Could not get output.");

		r = json_add(output_str);
		ERROR_CHECK_NEG(r, "Error while generating JSON.");
	}

	return 1;
}

int btk_privkey_set_compression(PrivKey key)
{
	static int last = OPTS_OUTPUT_COMPRESSION_TRUE;

	assert(key);

	switch (opts->compression)
	{
		case OPTS_OUTPUT_COMPRESSION_NONE:
			break;
		case OPTS_OUTPUT_COMPRESSION_TRUE:
			privkey_compress(key);
			break;
		case OPTS_OUTPUT_COMPRESSION_FALSE:
			privkey_uncompress(key);
			break;
		case OPTS_OUTPUT_COMPRESSION_BOTH:
			if (last == OPTS_OUTPUT_COMPRESSION_TRUE)
			{
				privkey_uncompress(key);
				last = OPTS_OUTPUT_COMPRESSION_FALSE;
			}
			else
			{
				privkey_compress(key);
				last = OPTS_OUTPUT_COMPRESSION_TRUE;
			}
			break;
	}

	return 1;
}

int btk_privkey_set_network(PrivKey key)
{
	assert(key);

	switch (opts->network)
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

	switch (opts->output_type)
	{
		case OPTS_OUTPUT_TYPE_WIF:
			r = privkey_to_wif(output, key);
			if (r < 0)
			{
				error_log("Could not convert private key to WIF format.");
				return -1;
			}
			break;
		case OPTS_OUTPUT_TYPE_HEX:
			r = privkey_to_hex(output, key, (opts->compression == OPTS_OUTPUT_COMPRESSION_TRUE || opts->compression == OPTS_OUTPUT_COMPRESSION_BOTH) ? 1 : 0);
			if (r < 0)
			{
				error_log("Could not convert private key to hex format.");
				return -1;
			}
			break;
		case OPTS_OUTPUT_TYPE_DECIMAL:
			r = privkey_to_dec(output, key);
			if (r < 0)
			{
				error_log("Could not convert private key to decimal format.");
				return -1;
			}
			break;
	}

	return 1;
}

int btk_privkey_output_hashes_process(char *input_str)
{
	int i, j;
	int output_hashes_len = 0;
	char *tok;
	char *tokend;
	long int tmp;

	// Save a pointer to the start of output_hashes so that it can be reset
	// for list processing.
	output_hashes_len = strlen(opts->rehashes);

	i = 0;
	tok = strtok(opts->rehashes, ",");
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
			if (opts->input_type != OPTS_INPUT_TYPE_STRING && opts->input_type != OPTS_INPUT_TYPE_DECIMAL)
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

	qsort(output_hashes_arr, i, sizeof(long int), btk_privkey_output_hashes_comp);

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
		if (opts->rehashes[i] == '\0')
		{
			opts->rehashes[i] = ',';
		}
	}

	return 1;
}

int btk_privkey_output_hashes_comp(const void *i, const void *j)
{
	return (*(long int *)i - *(long int *)j);
}

int btk_privkey_rehash(PrivKey key, int i)
{
	int r;
	long int hash_count;

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

	return 1;
}