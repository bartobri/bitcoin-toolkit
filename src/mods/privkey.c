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
#include <stdbool.h>
#include <assert.h>
#include <gmp.h>
#ifdef GMP_H_MISSING
#   include "GMP/mini-gmp.h"
#endif
#include "privkey.h"
#include "network.h"
#include "random.h"
#include "hex.h"
#include "base58.h"
#include "base58check.h"
#include "crypto.h"
#include "error.h"

#define MAINNET_PREFIX      0x80
#define TESTNET_PREFIX      0xEF

#define PRIVKEY_COMPRESSED_FLAG    0x01
#define PRIVKEY_UNCOMPRESSED_FLAG  0x00

struct PrivKey
{
	unsigned char data[PRIVKEY_LENGTH];
	int cflag;
};

int privkey_new(PrivKey key)
{
	int r;

	assert(key);

	r = random_get(key->data, PRIVKEY_LENGTH);
	if (r < 0)
	{
		error_log("Could not get random data for new private key.");
		return -1;
	}
	
	key->cflag = PRIVKEY_COMPRESSED_FLAG;
	
	return 1;
}

int privkey_compress(PrivKey key)
{
	assert(key);
	
	key->cflag = PRIVKEY_COMPRESSED_FLAG;
	
	return 1;
}

int privkey_uncompress(PrivKey key)
{
	assert(key);

	key->cflag = PRIVKEY_UNCOMPRESSED_FLAG;

	return 1;
}

int privkey_to_hex(char *str, PrivKey key, int cflag)
{
	int i;
	
	assert(key);
	assert(str);
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i)
	{
		sprintf(str + (i * 2), "%02x", key->data[i]);
	}

	if (cflag)
	{
		sprintf(str + (i * 2), "%02x", key->cflag);
		++i;
	}

	str[i * 2] = '\0';
	
	return 1;
}

int privkey_to_raw(unsigned char *raw, PrivKey key, int cflag)
{
	int len = PRIVKEY_LENGTH;

	assert(key);
	assert(raw);

	memcpy(raw, key->data, PRIVKEY_LENGTH);
	
	if (cflag)
	{
		raw[PRIVKEY_LENGTH] = key->cflag;
		len += 1;
	}

	return len;
}

int privkey_to_dec(char *str, PrivKey key)
{
	int r;
	char privkey_hex[(PRIVKEY_LENGTH * 2) + 1];
	mpz_t d;

	mpz_init(d);

	r = privkey_to_hex(privkey_hex, key, 0);
	if (r < 0)
	{
		error_log("Could not convert private key to hex string.");
		return -1;
	}

	privkey_hex[PRIVKEY_LENGTH * 2] = '\0';
	mpz_set_str(d, privkey_hex, 16);

	mpz_get_str(str, 10, d);

	return 1;
}

int privkey_to_wif(char *str, PrivKey key)
{
	int r, len;
	unsigned char p[PRIVKEY_LENGTH + 2];
	char *base58check;

	assert(str);
	assert(key);

	len = PRIVKEY_LENGTH + 1;

	if (network_is_main())
	{
		p[0] = MAINNET_PREFIX;
	}
	else if (network_is_test())
	{
		p[0] = TESTNET_PREFIX;
	}
	memcpy(p+1, key->data, PRIVKEY_LENGTH);
	if (privkey_is_compressed(key))
	{
		p[PRIVKEY_LENGTH + 1] = PRIVKEY_COMPRESSED_FLAG;
		len += 1;
	}

	// Assume the base58 string will never be longer
	// than twice the input string
	base58check = malloc(len * 2);
	if (base58check == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	r = base58check_encode(base58check, p, len);
	if (r < 0)
	{
		error_log("Could not encode private key to WIF format.");
		return -1;
	}

	strcpy(str, base58check);

	free(base58check);
	
	return 1;
}

int privkey_from_wif(PrivKey key, char *wif)
{
	unsigned char *p;
	int l;

	assert(key);
	assert(wif);

	// Assume that the length of the decoded data will always
	// be less than the encoded data
	p = malloc(strlen(wif) * 2);
	if (p == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	l = base58check_decode(p, wif, BASE58CHECK_TYPE_NA);
	if (l < 0)
	{
		error_log("Could not parse input string.");
		return -1;
	}

	if (l != PRIVKEY_LENGTH + 1 && l != PRIVKEY_LENGTH + 2)
	{
		error_log("Decoded input contains %i bytes. Valid byte length must be %i or %i.", l, PRIVKEY_LENGTH + 1, PRIVKEY_LENGTH + 2);
		return -1;
	}

	switch (p[0])
	{
		case MAINNET_PREFIX:
			network_set_main();
			break;
		case TESTNET_PREFIX:
			network_set_test();
			break;
		default:
			error_log("Input contains invalid network prefix.");
			return -1;
			break;
	}
	
	if (l == PRIVKEY_LENGTH + 2)
	{
		if (p[l - 1] == PRIVKEY_COMPRESSED_FLAG)
		{
			privkey_compress(key);
		}
		else
		{
			error_log("Input contains invalid compression flag.");
			return -1;
		}
	}
	else
	{
		privkey_uncompress(key);
	}

	memcpy(key->data, p+1, PRIVKEY_LENGTH);

	free(p);
	
	return 1;
}

int privkey_from_hex(PrivKey key, char *input)
{
	int r;
	size_t i, input_len;
	
	assert(input);
	assert(key);

	input_len = strlen(input);

	// Validating input string
	if (input_len % 2 != 0)
	{
		error_log("Input must contain an even number of characters to be valid hexidecimal.");
		return -1;
	}
	if (input_len < PRIVKEY_LENGTH * 2)
	{
		error_log("Input too short. Hexidecimal input must be at least %i characters in length.", PRIVKEY_LENGTH * 2);
		return -1;
	}

	// load input string as private key
	for (i = 0; i < PRIVKEY_LENGTH * 2; i += 2)
	{
		r = hex_to_dec(input[i], input[i+1]);
		if (r < 0)
		{
			error_log("Could not convert hexidecimal characters to decimal.");
			return -1;
		}
		key->data[i/2] = r;
	}

	if (input[i] && input[i+1])
	{
		r = hex_to_dec(input[i], input[i+1]);
		if (r < 0)
		{
			error_log("Could not convert hexidecimal characters to decimal.");
			return -1;
		}

		if (r == PRIVKEY_COMPRESSED_FLAG)
		{
			key->cflag = PRIVKEY_COMPRESSED_FLAG;
		}
		else if (r == PRIVKEY_UNCOMPRESSED_FLAG)
		{
			key->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
		}
		else
		{
			key->cflag = PRIVKEY_COMPRESSED_FLAG;
		}
	}
	else
	{
		key->cflag = PRIVKEY_COMPRESSED_FLAG;
	}

	return 1;
}

int privkey_from_dec(PrivKey key, char *data)
{
	int r;
	size_t i, c, data_len;
	mpz_t d;
	unsigned char *raw;
	PrivKey rawkey;

	assert(key);
	assert(data);

	data_len = strlen(data);
	for (i = 0; i < data_len; ++i)
	{
		if (data[i] < '0' || data[i] > '9')
		{
			error_log("Input contains non-decimal characters.");
			return -1;
		}
	}

	if (data[0] == '0')
	{
		error_log("Decimal input can not contain a leading zero.");
		return -1;
	}

	mpz_init(d);
	mpz_set_str(d, data, 10);
	i = (mpz_sizeinbase(d, 2) + 7) / 8;
	raw = malloc((i < PRIVKEY_LENGTH) ? PRIVKEY_LENGTH : i);
	if (raw == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	memset(raw, 0, (i < PRIVKEY_LENGTH) ? PRIVKEY_LENGTH : i);
	mpz_export(raw + PRIVKEY_LENGTH - i, &c, 1, 1, 1, 0, d);
	mpz_clear(d);

	rawkey = malloc(sizeof(*rawkey));
	if (rawkey == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	r = privkey_from_raw(rawkey, raw, PRIVKEY_LENGTH);
	if (r < 0)
	{
		error_log("Could not generate private key from input.");
		return -1;
	}

	memcpy(key->data, rawkey->data, PRIVKEY_LENGTH);
	
	free(rawkey);
	free(raw);

	privkey_compress(key);
	
	return 1;
}

int privkey_from_sbd(PrivKey key, char *data)
{
	size_t i;
	size_t data_len;

	assert(key);
	assert(data);

	data_len = strlen(data);

	if (data_len > PRIVKEY_LENGTH)
	{
		error_log("Input string too long for binary/decimal conversion.");
		return -1;
	}

	memset(key->data, 0, PRIVKEY_LENGTH);
	for (i = 0; i < data_len; ++i)
	{
		key->data[PRIVKEY_LENGTH - data_len + i] = data[i];
	}

	privkey_compress(key);

	return 1;
}

int privkey_from_str(PrivKey key, char *data)
{
	int r;

	assert(key);
	assert(data);

	r = crypto_get_sha256(key->data, (unsigned char*)data, strlen(data));
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash from input string.");
		return -1;
	}

	privkey_compress(key);
	
	return 1;
}

int privkey_from_raw(PrivKey key, unsigned char *raw, size_t l)
{
	assert(raw);
	assert(key);
	assert(l);

	if (l < PRIVKEY_LENGTH)
	{
		error_log("Length of input bytes (%i) is less than the minimum required (%i).", l, PRIVKEY_LENGTH);
		return -1;
	}

	memcpy(key->data, raw, PRIVKEY_LENGTH);

	if (l >= PRIVKEY_LENGTH + 1)
	{
		if (raw[PRIVKEY_LENGTH] == PRIVKEY_COMPRESSED_FLAG)
		{
			key->cflag = PRIVKEY_COMPRESSED_FLAG;
		}
		else if (raw[PRIVKEY_LENGTH] == PRIVKEY_UNCOMPRESSED_FLAG)
		{
			key->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
		}
		else
		{
			key->cflag = PRIVKEY_COMPRESSED_FLAG;
		}
	}
	else
	{
		key->cflag = PRIVKEY_COMPRESSED_FLAG;
	}
	
	return 1;
}

int privkey_from_blob(PrivKey key, unsigned char *data, size_t data_len)
{
	int r;

	assert(key);
	assert(data);
	assert(data_len);

	r = crypto_get_sha256(key->data, data, data_len);
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash from input.");
		return -1;
	}

	privkey_compress(key);
	
	return 1;
}

int privkey_from_guess(PrivKey key, unsigned char *data, size_t data_len)
{
	int r;
	size_t i;
	char data_str[BUFSIZ];

	assert(key);
	assert(data);
	assert(data_len);

	memset(data_str, 0, BUFSIZ);

	for (i = 0; i < data_len; ++i)
	{
		if (!isascii(data[i]))
		{
			break;
		}
	}
	if (i == data_len)
	{
		memcpy(data_str, data, data_len);
	}

	if (*data_str)
	{
		// Decimal
		r = privkey_from_dec(key, data_str);
		if (r > 0)
		{
			return PRIVKEY_GUESS_DECIMAL;
		}
		error_clear();

		// Hex
		r = privkey_from_hex(key, data_str);
		if (r > 0)
		{
			return PRIVKEY_GUESS_HEX;
		}
		error_clear();

		// WIF
		r = privkey_from_wif(key, data_str);
		if (r > 0)
		{
			return PRIVKEY_GUESS_WIF;
		}
		error_clear();

		// String
		r = privkey_from_str(key, data_str);
		if (r > 0)
		{
			return PRIVKEY_GUESS_STRING;
		}
		error_clear();
	}

	if (data_len == PRIVKEY_LENGTH)
	{
		// Raw
		r = privkey_from_raw(key, data, data_len);
		if (r > 0)
		{
			return PRIVKEY_GUESS_RAW;
		}
		error_clear();
	}
	else
	{
		// blob
		r = privkey_from_blob(key, data, data_len);
		if (r > 0)
		{
			return PRIVKEY_GUESS_BLOB;
		}
		error_clear();
	}
	

	error_log("Unable to guess input type.");
	return -1;
}

int privkey_is_compressed(PrivKey key)
{
	assert(key);
	return (key->cflag == PRIVKEY_COMPRESSED_FLAG) ? 1 : 0;
}

int privkey_is_zero(PrivKey key)
{
	int i;
	
	assert(key);

	for (i = 0; i < PRIVKEY_LENGTH; ++i)
		if (key->data[i] != 0)
			break;

	return i == PRIVKEY_LENGTH;
}

size_t privkey_sizeof(void)
{
	return sizeof(struct PrivKey);
}

int privkey_rehash(PrivKey key)
{
	int r;

	assert(key);

	r = crypto_get_sha256(key->data, key->data, PRIVKEY_LENGTH);
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash from key data.");
		return -1;
	}

	return 1;
}
