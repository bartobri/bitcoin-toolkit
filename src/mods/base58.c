/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <gmp.h>
#ifdef GMP_H_MISSING
#   include "GMP/mini-gmp.h"
#endif
#include "error.h"

#define BASE58_CODE_STRING_LENGTH 58

static char *code_string = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

int base58_encode(char *output, unsigned char *input, size_t input_len)
{
	int i, j;
	char t;
	mpz_t x, r, d;

	assert(output);
	assert(input);
	assert(input_len);

	mpz_init(x);
	mpz_init(r);
	mpz_init(d);

	mpz_set_ui(d, 58);

	// Base58 encode
	mpz_import(x, input_len, 1, sizeof(*input), 1, 0, input);
	for (i = 0; mpz_cmp_ui(x, 0) > 0; ++i)
	{
		mpz_tdiv_qr(x, r, x, d);
		output[i] = code_string[mpz_get_ui(r)];
	}
	for (j = 0; input[j] == 0; ++j, ++i)
	{
		output[i] = code_string[0];
	}
	output[i] = '\0';
	
	// Reverse result
	for (i = 0, j = strlen(output) - 1; i < j; ++i, --j)
	{
		t = output[i];
		output[i] = output[j];
		output[j] = t;
	}
	
	mpz_clear(x);
	mpz_clear(r);
	mpz_clear(d);

	return 1;
}

int base58_decode(unsigned char *output, char *input)
{
	int i, j;
	size_t r, input_len;
	mpz_t x, b;
	
	assert(input);
	assert(output);

	input_len = strlen(input);
	
	mpz_init(x);
	mpz_init(b);
	
	mpz_set_ui(b, 58);
	mpz_set_ui(x, 0);
	
	for (i = 0; i < (int)input_len; ++i)
	{
		for (j = 0; j < BASE58_CODE_STRING_LENGTH && code_string[j] != input[i]; ++j)
			;

		if (j >= BASE58_CODE_STRING_LENGTH)
		{
			error_log("Input contains invalid base58 character at index %i (0x%02x).", i, input[i]);
			return -1;
		}

		mpz_mul(x, b, x);
		mpz_add_ui(x, x, j);
	}

	mpz_export(output, &r, 1, 1, 1, 0, x);
	
	mpz_clear(x);
	mpz_clear(b);
	
	return (int)r;
}

int base58_ischar(char c)
{
	int i;

	for (i = 0; i < BASE58_CODE_STRING_LENGTH && code_string[i] != c; ++i)
		;

	return (i < BASE58_CODE_STRING_LENGTH);
}
