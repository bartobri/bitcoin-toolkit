/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#ifdef GMP_H_MISSING
#   include "GMP/mini-gmp.h"
#endif
#include "pubkey.h"
#include "privkey.h"
#include "point.h"
#include "crypto.h"
#include "base58check.h"
#include "hex.h"
#include "network.h"
#include "error.h"

#define PUBKEY_COMPRESSED_FLAG_EVEN   0x02
#define PUBKEY_COMPRESSED_FLAG_ODD    0x03
#define PUBKEY_UNCOMPRESSED_FLAG      0x04
#define PUBKEY_POINTS                 (PRIVKEY_LENGTH * 8)

struct PubKey
{
	unsigned char data[PUBKEY_UNCOMPRESSED_LENGTH + 1];
};

int pubkey_get(PubKey pubkey, PrivKey privkey)
{
	int r;
	size_t i, l;
	char privkey_hex[((PRIVKEY_LENGTH + 1) * 2) + 1];
	mpz_t bignum;
	Point point;
	Point points[PUBKEY_POINTS];
	
	assert(privkey);
	assert(pubkey);

	memset(pubkey->data, 0, PUBKEY_UNCOMPRESSED_LENGTH + 1);

	if (privkey_is_zero(privkey))
	{
		error_log("Private key can not be zero.");
		return -1;
	}

	mpz_init(bignum);

	// Load private key from hex string.
	r = privkey_to_hex(privkey_hex, privkey, 0);
	if (r < 0)
	{
		error_log("Could not convert private key to hex string.");
		return -1;
	}
	privkey_hex[PRIVKEY_LENGTH * 2] = '\0';
	mpz_set_str(bignum, privkey_hex, 16);
	
	// Initalize the points
	point = malloc(sizeof(*point));
	if (point == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	point_init(point);
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		points[i] = malloc(sizeof(*point));
		if (points[i] == NULL)
		{
			error_log("Memory allocation error.");
			return -1;
		}
		point_init(points[i]);
	}

	// Calculating public key
	point_set_generator(points[0]);
	for (i = 1; i < PUBKEY_POINTS; ++i)
	{
		point_double(points[i], points[i-1]);
		if (!point_verify(points[i]))
		{
			error_log("Unexpected point value while calculating public key.");
			return -1;
		}
	}

	// Add all points corresponding to 1 bits
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		if (mpz_tstbit(bignum, i) == 1)
		{
			if (mpz_cmp_ui(point->x, 0) == 0 && mpz_cmp_ui(point->y, 0) == 0)
			{
				point_set(point, points[i]);
			}
			else
			{
				point_add(point, point, points[i]);
			}
			if (!point_verify(points[i]))
			{
				error_log("Unexpected point value while calculating public key.");
				return -1;
			}
		}
	}
	
	// Setting compression flag
	if (privkey_is_compressed(privkey))
	{
		if (mpz_even_p(point->y))
		{
			pubkey->data[0] = PUBKEY_COMPRESSED_FLAG_EVEN;
		}
		else
		{
			pubkey->data[0] = PUBKEY_COMPRESSED_FLAG_ODD;
		}
	}
	else
	{
		pubkey->data[0] = PUBKEY_UNCOMPRESSED_FLAG;
	}
	
	// Exporting x,y coordinates as byte string, making sure to leave leading
	// zeros if either exports as less than 32 bytes.
	l = (mpz_sizeinbase(point->x, 2) + 7) / 8;
	mpz_export(pubkey->data + 1 + (32 - l), &i, 1, 1, 1, 0, point->x);
	if (l != i)
	{
		error_log("Length of public key x-value export (%zu) does not match expected length (%zu).", i, l);
		return -1;
	}
	if (!privkey_is_compressed(privkey))
	{
		l = (mpz_sizeinbase(point->y, 2) + 7) / 8;
		mpz_export(pubkey->data + 33 + (32 - l), &i, 1, 1, 1, 0, point->y);
		if (l != i)
		{
			error_log("Length of public key y-value export (%zu) does not match expected length (%zu).", i, l);
			return -1;
		}
	}

	// Clear all mpz data
	mpz_clear(bignum);
	point_clear(point);
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		point_clear(points[i]);
	}

	// Free all points
	free(point);
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		free(points[i]);
	}

	return 1;
}

int pubkey_from_hex(PubKey key, char *input)
{
	int r;
	size_t input_len;
	unsigned char *raw_input;
	
	assert(input);
	assert(key);

	input_len = strlen(input);

	if (input_len % 2 != 0)
	{
		error_log("Input must contain an even number of characters to be valid hexidecimal.");
		return -1;
	}

	raw_input = malloc(input_len / 2);
	if (raw_input == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = hex_str_to_raw(raw_input, input);
	if (r < 0)
	{
		error_log("Could not convert hex input to raw binary.");
		return -1;
	}

	r = pubkey_from_raw(key, raw_input, input_len / 2);
	if (r < 0)
	{
		error_log("Can't convert raw input to public key.");
		return -1;
	}

	free(raw_input);

	return 1;
}

int pubkey_from_raw(PubKey key, unsigned char *input, size_t input_len)
{
	assert(key);
	assert(input);
	assert(input_len);

	if (input[0] != PUBKEY_COMPRESSED_FLAG_EVEN && input[0] != PUBKEY_COMPRESSED_FLAG_ODD && input[0] != PUBKEY_UNCOMPRESSED_FLAG)
	{
		error_log("Unknown compression flag: %.2x", input[0]);
		return -1;
	}

	if ((input[0] == PUBKEY_COMPRESSED_FLAG_EVEN || input[0] == PUBKEY_COMPRESSED_FLAG_ODD) && input_len != PUBKEY_COMPRESSED_LENGTH + 1)
	{
		error_log("Incorrect data length for compressed public key.");
		return -1;
	}

	if (input[0] == PUBKEY_UNCOMPRESSED_FLAG && input_len != PUBKEY_UNCOMPRESSED_LENGTH + 1)
	{
		error_log("Incorrect data length for uncompressed public key: %zu", input_len);
		return -1;
	}

	memset(key->data, 0, PUBKEY_UNCOMPRESSED_LENGTH + 1);

	memcpy(key->data, input, input_len);

	return 1;
}

int pubkey_from_guess(PubKey key, unsigned char *input, size_t input_len)
{
	int r;
	size_t i;
	char input_str[BUFSIZ];
	PrivKey privkey;

	assert(key);
	assert(input);
	assert(input_len);

	memset(input_str, 0, BUFSIZ);

	for (i = 0; i < input_len; i++)
	{
		if (!isascii(input[i]))
		{
			break;
		}
	}
	if (i > 0)
	{
		memcpy(input_str, input, input_len);
	}

	if (*input_str)
	{
		privkey = malloc(privkey_sizeof());
		ERROR_CHECK_NULL(privkey, "Memory allocation error.");

		r = privkey_from_wif(privkey, input_str);
		if (r > 0)
		{
			r = pubkey_get(key, privkey);
			if (r > 0)
			{
				free(privkey);
				return 1;
			}
		}

		free(privkey);
		error_clear();

		r = pubkey_from_hex(key, input_str);
		if (r > 0)
		{
			return 1;
		}

		error_clear();
	}

	error_log("Unable to guess input type. Specify an input type flag.");
	return -1;
}

int pubkey_compress(PubKey key)
{
	mpz_t y;
	size_t point_length = 32;
	
	assert(key);
	
	if (key->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || key->data[0] == PUBKEY_COMPRESSED_FLAG_ODD)
	{
		return 1;
	}

	mpz_init(y);

	mpz_import(y, point_length, 1, 1, 1, 0, key->data + 1 + point_length);
	
	if (mpz_even_p(y))
	{
		key->data[0] = PUBKEY_COMPRESSED_FLAG_EVEN;
	}
	else
	{
		key->data[0] = PUBKEY_COMPRESSED_FLAG_ODD;
	}

	mpz_clear(y);
	
	return 1;
}

int pubkey_uncompress(PubKey key)
{
	size_t i, l;
	Point point;

	if (key->data[0] == PUBKEY_UNCOMPRESSED_FLAG)
	{
		return 1;
	}

	if (key->data[0] != PUBKEY_COMPRESSED_FLAG_EVEN && key->data[0] != PUBKEY_COMPRESSED_FLAG_ODD)
	{
		error_log("Unknown compression flag: %.2x", key->data[0]);
		return -1;
	}

	point = malloc(sizeof(*point));
	if (point == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	point_init(point);

	mpz_import(point->x, PUBKEY_COMPRESSED_LENGTH, 1, 1, 1, 0, key->data + 1);

	point_solve_y(point, key->data[0]);

	if (!point_verify(point))
	{
		error_log("Invalid point values.");
		return -1;
	}

	memset(key->data + 33, 0, 32);

	l = (mpz_sizeinbase(point->y, 2) + 7) / 8;
	mpz_export(key->data + 33 + (32 - l), &i, 1, 1, 1, 0, point->y);
	if (l != i)
	{
		error_log("Length of public key y-value export (%zu) does not match expected length (%zu).", i, l);
		return -1;
	}

	key->data[0] = PUBKEY_UNCOMPRESSED_FLAG;

	point_clear(point);
	free(point);

	return 1;
}

int pubkey_is_compressed(PubKey key)
{
	assert(key);

	return (key->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || key->data[0] == PUBKEY_COMPRESSED_FLAG_ODD);
}

int pubkey_to_hex(char *str, PubKey key)
{
	int i, l;
	
	assert(str);
	assert(key);
	
	switch (key->data[0])
	{
		case PUBKEY_UNCOMPRESSED_FLAG:
			l = (PUBKEY_UNCOMPRESSED_LENGTH * 2) + 2;
			break;
		case PUBKEY_COMPRESSED_FLAG_EVEN:
		case PUBKEY_COMPRESSED_FLAG_ODD:
			l = (PUBKEY_COMPRESSED_LENGTH * 2) + 2;
			break;
		default:
			error_log("Public key contains invalid compression flag.");
			return -1;
	}
	
	for (i = 0; i < l; i += 2)
	{
		sprintf(str + i, "%02x", key->data[i/2]);
	}
	str[l] = '\0';
	
	return 1;
}

int pubkey_to_raw(unsigned char *raw, PubKey key)
{
	int i, l;

	assert(raw);
	assert(key);
	
	l = pubkey_is_compressed(key) ? PUBKEY_COMPRESSED_LENGTH + 1 : PUBKEY_UNCOMPRESSED_LENGTH + 1;

	for (i = 0; i < l; ++i)
	{
		raw[i] = key->data[i];
	}
	
	return l;
}

size_t pubkey_sizeof(void)
{
	return sizeof(struct PubKey);
}

