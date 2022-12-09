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

// Input Type
#define INPUT_NEW               1
#define INPUT_WIF               2
#define INPUT_HEX               3
#define INPUT_RAW               4
#define INPUT_STR               5
#define INPUT_DEC               6
#define INPUT_BLOB              7
#define INPUT_GUESS             8
#define INPUT_SBD               9
// Input Format
#define INPUT_JSON              1
#define INPUT_ASCII             2
#define INPUT_BINARY            3
// Output Type
#define OUTPUT_WIF              1
#define OUTPUT_HEX              2
#define OUTPUT_DEC              3
// Output format
#define OUTPUT_JSON             1
#define OUTPUT_LIST             2
// Output Compression
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define OUTPUT_COMPRESSION_BOTH 3
// Output Network
#define OUTPUT_MAINNET          1
#define OUTPUT_TESTNET          2
// MISC
#define TRUE                    1
#define FALSE                   0
#define OUTPUT_BUFFER           150
#define OUTPUT_HASH_MAX         100
#define HASH_WILDCARD           "w"

#define INPUT_SET_FORMAT(x)     if (input_format == FALSE) { input_format = x; } else { error_log("Cannot specify ascii input format for binary input types."); return -1; }
#define INPUT_SET(x)            if (input_type == FALSE) { input_type = x; } else { error_log("Cannot use multiple input type flags."); return -1; }
#define OUTPUT_SET(x)           if (output_type == FALSE) { output_type = x; } else { error_log("Cannot use multiple output type flags."); return -1; }
#define COMPRESSION_SET(x)      if (output_compression == FALSE) { output_compression = x; } else { output_compression = OUTPUT_COMPRESSION_BOTH; }

int btk_privkey_input_to_json(unsigned char **, size_t *);
int btk_privkey_get(PrivKey, char *, unsigned char *, size_t);
int btk_privkey_hashes_args_add(PrivKey, char *);
int btk_privkey_args_add(PrivKey);
int btk_privkey_set_compression(PrivKey);
int btk_privkey_set_network(PrivKey);
int btk_privkey_to_output(char *, PrivKey);
int btk_privkey_output_hashes_process(char *);
int btk_privkey_output_hashes_comp(const void *, const void *);
int btk_privkey_rehash(PrivKey, int);

static int input_format       = FALSE;
static int input_type         = FALSE;
static int output_type        = FALSE;
static int output_compression = FALSE;
static int output_network     = FALSE;
static char *output_hashes    = NULL;
static long int output_hashes_arr[OUTPUT_HASH_MAX];
static int output_hashes_arr_len = 0;

int btk_privkey_init(int argc, char *argv[])
{
	int o;
	char *command = NULL;

	command = argv[1];

	while ((o = getopt(argc, argv, "nwhrsdbxaWHCUTDMS:")) != -1)
	{
		switch (o)
		{
			// Input type
			case 'n':
				INPUT_SET(INPUT_NEW);
				break;
			case 'w':
				INPUT_SET(INPUT_WIF);
				break;
			case 'h':
				INPUT_SET(INPUT_HEX);
				break;
			case 'r':
				INPUT_SET(INPUT_RAW);
				INPUT_SET_FORMAT(INPUT_BINARY);
				break;
			case 's':
				INPUT_SET(INPUT_STR);
				break;
			case 'd':
				INPUT_SET(INPUT_DEC);
				break;
			case 'b':
				INPUT_SET(INPUT_BLOB);
				INPUT_SET_FORMAT(INPUT_BINARY);
				break;
			case 'x':
				INPUT_SET(INPUT_SBD);
				break;

			// Input Format
			case 'a':
				INPUT_SET_FORMAT(INPUT_ASCII);
				break;

			// Output type
			case 'W':
				OUTPUT_SET(OUTPUT_WIF);
				break;
			case 'H':
				OUTPUT_SET(OUTPUT_HEX);
				break;
			case 'D':
				OUTPUT_SET(OUTPUT_DEC);
				break;
			
			// Output Compression
			case 'C':
				COMPRESSION_SET(OUTPUT_COMPRESS);
				break;
			case 'U':
				COMPRESSION_SET(OUTPUT_UNCOMPRESS);
				break;

			// Network Options
			case 'T':
				output_network = OUTPUT_TESTNET;
				break;
			case 'M':
				output_network = OUTPUT_MAINNET;
				break;

			// Other options
			case 'S':
				output_hashes = optarg;
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

	if (input_format == FALSE)
	{
		input_format = INPUT_JSON;
	}

	if (output_type == FALSE)
	{
		output_type = OUTPUT_WIF;
	}

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

	r = input_get(&input, &input_len);
	ERROR_CHECK_NEG(r, "Error getting input.");

	json_init();

	if (input_format == INPUT_ASCII)
	{
		r = btk_privkey_input_to_json(&input, &input_len);
		ERROR_CHECK_NEG(r, "Could not convert input to JSON.");

		input_format = INPUT_JSON;
	}

	if (input_format == INPUT_JSON)
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

				r = json_get_input_index(input_str, i);
				ERROR_CHECK_NEG(r, "Could not get JSON string object at index.");

				r = btk_privkey_get(key, input_str, NULL, 0);
				ERROR_CHECK_NEG(r, "Could not get privkey from input.");

				if (output_hashes != NULL)
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
	else if (input_format == INPUT_BINARY)
	{
		r = btk_privkey_get(key, NULL, input, input_len);
		ERROR_CHECK_NEG(r, "Could not get privkey from input.");

		if (output_hashes != NULL)
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

	json_print();
	json_free();

	free(key);

	if (input != NULL)
	{
		free(input);
	}

	return 1;
}

int btk_privkey_cleanup(void)
{
	return 1;
}

int btk_privkey_input_to_json(unsigned char **input, size_t *input_len)
{
	int r;
	size_t i;
	char str[BUFSIZ];

	memset(str, 0, BUFSIZ);

	for (i = 0; i < *input_len; i++)
	{
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

	free(*input);

	r = json_from_string((char **)input, str);
	ERROR_CHECK_NEG(r, "Could not convert input to JSON.");

	*input_len = strlen((char *)*input);

	return 1;
}

int btk_privkey_get(PrivKey key, char *sc_input, unsigned char *uc_input, size_t uc_input_len)
{
	int r;

	assert(key);
	assert(sc_input || uc_input);

	switch (input_type)
	{
		case INPUT_NEW:
			r = privkey_new(key);
			ERROR_CHECK_NEG(r, "Could not generate a new private key.");
			break;
		case INPUT_WIF:
			r = privkey_from_wif(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_HEX:
			r = privkey_from_hex(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_STR:
			r = privkey_from_str(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_DEC:
			r = privkey_from_dec(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_SBD:
			r = privkey_from_sbd(key, sc_input);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_RAW:
			r = privkey_from_raw(key, uc_input, uc_input_len);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_BLOB:
			r = privkey_from_blob(key, uc_input, uc_input_len);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
		case INPUT_GUESS:
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
	if (r == 0)
	{
		// TODO - check that this logic is correct
		// Maybe i should treat this as a zero rehash instead.
		return 0;
	}

	for(i = 0; i < output_hashes_arr_len; i++)
	{
		r = btk_privkey_rehash(key, i);
		ERROR_CHECK_NEG(r, "Unable to rehash private key.");

		r = btk_privkey_args_add(key);
		ERROR_CHECK_NEG(r, "");
	}

	return 1;
}

int btk_privkey_args_add(PrivKey key)
{
	int r;
	char output_str[BUFSIZ];

	memset(output_str, 0, BUFSIZ);

	r = btk_privkey_set_network(key);
	ERROR_CHECK_NEG(r, "Could not set key network.");

	r = btk_privkey_set_compression(key);
	ERROR_CHECK_NEG(r, "Could not set key compression.");

	r = btk_privkey_to_output(output_str, key);
	ERROR_CHECK_NEG(r, "Could not get output.");

	r = json_add(output_str);
	ERROR_CHECK_NEG(r, "Error while generating JSON.");

	if (output_compression == OUTPUT_COMPRESSION_BOTH)
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
	static int last = OUTPUT_COMPRESS;

	assert(key);

	switch (output_compression)
	{
		case FALSE:
			break;
		case OUTPUT_COMPRESS:
			privkey_compress(key);
			break;
		case OUTPUT_UNCOMPRESS:
			privkey_uncompress(key);
			break;
		case OUTPUT_COMPRESSION_BOTH:
			if (last == OUTPUT_COMPRESS)
			{
				privkey_uncompress(key);
				last = OUTPUT_UNCOMPRESS;
			}
			else
			{
				privkey_compress(key);
				last = OUTPUT_COMPRESS;
			}
			break;
	}

	return 1;
}

int btk_privkey_set_network(PrivKey key)
{
	assert(key);

	switch (output_network)
	{
		case FALSE:
			break;
		case OUTPUT_MAINNET:
			network_set_main();
			break;
		case OUTPUT_TESTNET:
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

	switch (output_type)
	{
		case OUTPUT_WIF:
			r = privkey_to_wif(output, key);
			if (r < 0)
			{
				error_log("Could not convert private key to WIF format.");
				return -1;
			}
			break;
		case OUTPUT_HEX:
			r = privkey_to_hex(output, key, output_compression);
			if (r < 0)
			{
				error_log("Could not convert private key to hex format.");
				return -1;
			}
			break;
		case OUTPUT_DEC:
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
	output_hashes_len = strlen(output_hashes);

	i = 0;
	tok = strtok(output_hashes, ",");
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
			if (input_type != INPUT_STR && input_type != INPUT_DEC)
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
		if (output_hashes[i] == '\0')
		{
			output_hashes[i] = ',';
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