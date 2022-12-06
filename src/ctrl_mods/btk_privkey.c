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
#define INPUT_LIST              2
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

#define INPUT_SET(x)            if (input_type == FALSE) { input_type = x; } else { error_log("Cannot use multiple input type flags."); return -1; }
#define OUTPUT_SET(x)           if (output_type == FALSE) { output_type = x; } else { error_log("Cannot use multiple output type flags."); return -1; }
#define COMPRESSION_SET(x)      if (output_compression == FALSE) { output_compression = x; } else { output_compression = OUTPUT_COMPRESSION_BOTH; }

int btk_privkey_json_get(PrivKey, char *);
int btk_privkey_binary_get(PrivKey, unsigned char*, size_t);
int btk_privkey_set_compression(PrivKey);
int btk_privkey_set_network(PrivKey);
int btk_privkey_to_output(char *, PrivKey);
int btk_privkey_output_hashes_process(char *);
int btk_privkey_output_hashes_comp(const void *, const void *);
int btk_privkey_rehash(PrivKey, int);

static int input_type         = FALSE;
static int input_format       = FALSE;
static int output_type        = FALSE;
static int output_compression = FALSE;
static int output_network     = FALSE;
static char *output_hashes    = NULL;
static char *output_hashes_arr[OUTPUT_HASH_MAX];

int btk_privkey_init(int argc, char *argv[])
{
	int o, i;
	char *command = NULL;

	command = argv[1];

	while ((o = getopt(argc, argv, "nwhrsdbxWHRCUTDMS:")) != -1)
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
				input_format = INPUT_BINARY;
				break;
			case 's':
				INPUT_SET(INPUT_STR);
				break;
			case 'd':
				INPUT_SET(INPUT_DEC);
				break;
			case 'b':
				INPUT_SET(INPUT_BLOB);
				input_format = INPUT_BINARY;
				break;
			case 'x':
				INPUT_SET(INPUT_SBD);
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

			// Other options
			case 'S':
				output_hashes = optarg;
				break;

			// Network Options
			case 'T':
				output_network = OUTPUT_TESTNET;
				break;
			case 'M':
				output_network = OUTPUT_MAINNET;
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

	for (i = 0; i < OUTPUT_HASH_MAX; i++)
	{
		output_hashes_arr[i] = NULL;
	}

	return 1;
}

int btk_privkey_main(void)
{
	int r, i, j, len = 0;
	PrivKey key = NULL;
	unsigned char *input; 
	size_t input_len;
	char input_str[BUFSIZ];
	char output_str[BUFSIZ];

	key = malloc(privkey_sizeof());
	ERROR_CHECK_NULL(key, "Memory allocation error.");

	r = input_get(&input, &input_len);
	ERROR_CHECK_NEG(r, "Error getting input.");

	json_init();

	if (input_format == INPUT_JSON)
	{
		if(json_is_valid((char *)input))
		{
			r = json_set_input((char *)input);
			ERROR_CHECK_NEG(r, "Could not load JSON input.");

			r = json_get_input_len(&len);
			ERROR_CHECK_NEG(r, "Could not get input list length.");

			for (i = 0; i < len; i++)
			{
				memset(input_str, 0, BUFSIZ);
				memset(output_str, 0, BUFSIZ);

				r = json_get_input_index(input_str, i);
				ERROR_CHECK_NEG(r, "Could not get JSON string object at index.");

				r = btk_privkey_json_get(key, input_str);
				ERROR_CHECK_NEG(r, "Could not get privkey from input.");

				r = btk_privkey_set_network(key);
				ERROR_CHECK_NEG(r, "Could not set key network.");

				if (output_hashes != NULL)
				{
					r = btk_privkey_output_hashes_process(input_str);
					ERROR_CHECK_NEG(r, "Error while processing hash argument [-S].");
					if (r == 0)
					{
						// TODO - check that this logic is correct
						// Maybe i should treat this as a zero rehash instead.
						continue;
					}

					for(j = 0; output_hashes_arr[j] != NULL; j++)
					{
						memset(output_str, 0, BUFSIZ);

						r = btk_privkey_rehash(key, j);
						ERROR_CHECK_NEG(r, "Unable to rehash private key.");

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
					}
				}
				else
				{
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
				}
			}
		}
		else
		{
			error_log("Invalid JSON. Input must be in JSON format.");
			return -1;
		}
	}
	else if (input_format == INPUT_BINARY)
	{
		memset(output_str, 0, BUFSIZ);

		r = btk_privkey_binary_get(key, input, input_len);
		ERROR_CHECK_NEG(r, "Could not get privkey from input.");

		r = btk_privkey_set_compression(key);
		ERROR_CHECK_NEG(r, "Could not set key compression.");
		
		r = btk_privkey_set_network(key);
		ERROR_CHECK_NEG(r, "Could not set key network.");

		if (output_hashes != NULL)
		{
			r = btk_privkey_output_hashes_process(input_str);
			ERROR_CHECK_NEG(r, "Error while processing hash argument [-S].");

			for(j = 0; output_hashes_arr[j] != NULL; j++)
			{
				memset(output_str, 0, BUFSIZ);

				r = btk_privkey_rehash(key, j);
				ERROR_CHECK_NEG(r, "Unable to rehash private key.");

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
			}
		}
		else
		{
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

int btk_privkey_json_get(PrivKey key, char *input)
{
	int r;

	assert(key);
	assert(input);

	switch (input_type)
	{
		case INPUT_NEW:
			r = privkey_new(key);
			if (r < 0)
			{
				error_log("Could not generate a new private key.");
				return -1;
			}
			break;
		case INPUT_WIF:
			r = privkey_from_wif(key, input);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}
			break;
		case INPUT_HEX:
			r = privkey_from_hex(key, input);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}
			break;
		case INPUT_STR:
			r = privkey_from_str(key, input);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}
			break;
		case INPUT_DEC:
			r = privkey_from_dec(key, input);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}
			break;
		case INPUT_SBD:
			r = privkey_from_sbd(key, input);
			if (r < 0)
			{
				error_log("Could not calculate private key from input.");
				return -1;
			}
			break;
		case INPUT_GUESS:
			error_log("Re-implement guess option to set input type before we get this far.");
			return -1;
	}

	if (!key)
	{
		error_log("Could not calculate private key from input.");
		return -1;
	}

	if (privkey_is_zero(key))
	{
		error_log("Key value cannot be zero.");
		return -1;
	}

	return 1;
}

int btk_privkey_binary_get(PrivKey key, unsigned char *input, size_t input_len)
{
	int r;

	assert(key);
	assert(input);

	switch (input_type)
	{
		case INPUT_RAW:
			r = privkey_from_raw(key, input, input_len);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;

		case INPUT_BLOB:
			r = privkey_from_blob(key, input, input_len);
			ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
			break;
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
	size_t i, j, len, N = 0;
	size_t output_hashes_len = 0;
	char *wild = NULL;

	// Save a pointer to the start of output_hashes so that it can be reset
	// after processing.
	output_hashes_len = strlen(output_hashes);

	if (output_hashes != NULL)
	{
		// Parsing string
		output_hashes_arr[N] = strtok(output_hashes, ",");
		while (output_hashes_arr[N] != NULL)
		{
			output_hashes_arr[++N] = strtok(NULL, ",");
		}

		// Valid token check
		for (i = 0; i < N; i++)
		{
			if (output_hashes_arr[i][0] == '-')
			{
				error_log("Argument cannot contain a negative number: %s", output_hashes_arr[i]);
				return -1;
			}
			else if (strcmp(output_hashes_arr[i], HASH_WILDCARD) == 0)
			{
				if (input_type != INPUT_STR && input_type != INPUT_DEC)
				{
					error_log("Can not use wildcard '%s' with current input mode.", output_hashes_arr[i]);
					return -1;
				}

				for (j = strlen(input_str); isdigit(input_str[j-1]); j--)
				{
					if (j == 0)
					{
						break;
					}
				}
				if (strlen(input_str + j) > 0)
				{
					wild = input_str + j;
				}
			}
			else
			{
				len = strlen(output_hashes_arr[i]);
				for (j = 0; j < len; j++)
				{
					if (!isdigit(output_hashes_arr[i][j]))
					{
						error_log("Argument contains unexpected character: %c", output_hashes_arr[i][j]);
						return -1;
					}
				}
			}
		}

		// Max number for wildcard is 1000000. Ignore anything greater.
		if (wild != NULL && atoi(wild) > 1000000)
		{
			wild = NULL;
		}

		// Duplicate check
		for (i = 0; i < N; i++)
		{
			for (j = i + 1; j < N; j++)
			{
				if (strcmp(output_hashes_arr[i], output_hashes_arr[j]) == 0)
				{
					error_log("Argument cannot contain duplicate numbers: %s", output_hashes_arr[i]);
					return -1;
				}
			}
			if (wild != NULL && strcmp(output_hashes_arr[i], wild) == 0)
			{
				wild = NULL;
			}
		}

		// Sort
		qsort(output_hashes_arr, N, sizeof(char *), btk_privkey_output_hashes_comp);

		// If all we have is a wildcard and no wildcard value,
		// return zero which tells the caller not to print any output.
		if (strcmp(output_hashes_arr[0], HASH_WILDCARD) == 0 && wild == NULL)
		{
			return 0;
		}

		// Wildcard substitute
		if (strcmp(output_hashes_arr[N-1], HASH_WILDCARD) == 0)
		{
			if (wild != NULL)
			{
				output_hashes_arr[N-1] = wild;
			}
			else
			{
				output_hashes_arr[N-1] = NULL;
			}
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
	if (strcmp(*(char **)i, HASH_WILDCARD) == 0)
	{
		return 1;
	}
	if (strcmp(*(char **)j, HASH_WILDCARD) == 0)
	{
		return -1;
	}
	return (atoi(*(char **)i) - atoi(*(char **)j));
}

int btk_privkey_rehash(PrivKey key, int i)
{
	int r, hash_count;

	hash_count = atoi(output_hashes_arr[i]);
	if (i > 0)
	{
		hash_count -= atoi(output_hashes_arr[i-1]);
	}

	while (hash_count > 0)
	{
		r = privkey_rehash(key);
		ERROR_CHECK_NEG(r, "Unable to rehash private key.");

		hash_count--;
	}

	return 1;
}