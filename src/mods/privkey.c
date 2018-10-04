#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gmp.h>
#include "privkey.h"
#include "network.h"
#include "random.h"
#include "hex.h"
#include "base58.h"
#include "base58check.h"
#include "crypto.h"
#include "error.h"
#include "mem.h"
#include "assert.h"

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
		error_log("Error while getting random data for new private key.");
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

int privkey_to_hex(char *str, PrivKey key)
{
	int i;
	
	assert(key);
	assert(str);
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		sprintf(str + (i * 2), "%02x", key->data[i]);
	}
	if (key->cflag == PRIVKEY_COMPRESSED_FLAG) {
		sprintf(str + (i * 2), "%02x", key->cflag);
	}
	str[++i * 2] = '\0';
	
	return 1;
}

int privkey_to_raw(unsigned char *raw, PrivKey key)
{
	int len = PRIVKEY_LENGTH;

	assert(key);
	assert(raw);

	memcpy(raw, key->data, PRIVKEY_LENGTH);
	
	if (privkey_is_compressed(key)) {
		raw[PRIVKEY_LENGTH] = PRIVKEY_COMPRESSED_FLAG;
		len += 1;
	}

	return len;
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
	base58check = ALLOC(len * 2);
	r = base58check_encode(base58check, p, len);
	if (r < 0)
	{
		error_log("Error while encoding private key to WIF format.");
		return -1;
	}

	strcpy(str, base58check);

	FREE(base58check);
	
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
	p = ALLOC(strlen(wif) * 2);

	l = base58check_decode(p, wif);
	if (l < 0)
	{
		error_log("Error while decoding private key from WIF format.");
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

	FREE(p);
	
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
			error_log("Error while converting hexidecimal characters to decimal.");
			return -1;
		}
		key->data[i/2] = r;
	}

	key->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
	if (input[i] && input[i+1])
	{
		r = hex_to_dec(input[i], input[i+1]);
		if (r < 0)
		{
			error_log("Error while converting hexidecimal characters to decimal.");
			return -1;
		}
		if (r == PRIVKEY_COMPRESSED_FLAG)
		{
			key->cflag = PRIVKEY_COMPRESSED_FLAG;
		}
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

	mpz_init(d);
	mpz_set_str(d, data, 10);
	i = (mpz_sizeinbase(d, 2) + 7) / 8;
	raw = ALLOC((i < PRIVKEY_LENGTH) ? PRIVKEY_LENGTH : i);
	memset(raw, 0, (i < PRIVKEY_LENGTH) ? PRIVKEY_LENGTH : i);
	mpz_export(raw + PRIVKEY_LENGTH - i, &c, 1, 1, 1, 0, d);
	mpz_clear(d);

	NEW(rawkey);
	r = privkey_from_raw(rawkey, raw, PRIVKEY_LENGTH);
	if (r < 0)
	{
		error_log("Error generating private key from input.");
		return -1;
	}

	memcpy(key->data, rawkey->data, PRIVKEY_LENGTH);
	
	FREE(rawkey);
	FREE(raw);

	privkey_compress(key);
	
	return 1;
}

int privkey_from_str(PrivKey key, char *data)
{
	int r;
	unsigned char *tmp;

	assert(key);
	assert(data);
	
	tmp = ALLOC(32);

	r = crypto_get_sha256(tmp, (unsigned char*)data, strlen(data));
	if (r < 0)
	{
		error_log("Error generating SHA256 hash from input string.");
		return -1;
	}
	memcpy(key->data, tmp, PRIVKEY_LENGTH);
	FREE(tmp);

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
	
	if (l >= PRIVKEY_LENGTH + 1 && raw[PRIVKEY_LENGTH] == PRIVKEY_COMPRESSED_FLAG)
		key->cflag = PRIVKEY_COMPRESSED_FLAG;
	else
		key->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
	
	return 1;
}

int privkey_from_blob(PrivKey key, unsigned char *data, size_t data_len)
{
	int r;
	unsigned char *tmp;

	assert(key);
	assert(data);
	assert(data_len);
	
	tmp = ALLOC(32);

	r = crypto_get_sha256(tmp, data, data_len);
	if (r < 0)
	{
		error_log("Error generating SHA256 hash from input.");
		return -1;
	}
	memcpy(key->data, tmp, PRIVKEY_LENGTH);
	free(tmp);

	privkey_compress(key);
	
	return 1;
}

int privkey_from_guess(PrivKey key, unsigned char *data, size_t data_len)
{
	size_t i;
	int r;
	char *data_str;

	assert(key);
	assert(data);
	assert(data_len);

	data_str = NULL;
	for (i = 0; i < data_len; ++i)
	{
		if (!isascii(data[i]))
		{
			break;
		}
	}
	if (i == data_len)
	{
		while (isspace(data[data_len - 1]))
		{
			--data_len;
		}
		data_str = ALLOC(data_len + 1);
		memcpy(data_str, data, data_len);
		data_str[data_len] = '\0';
	}

	if (data_str != NULL)
	{
		// Decimal
		r = privkey_from_dec(key, data_str);
		if (r > 0)
		{
			return 1;
		}

		// Hex
		r = privkey_from_hex(key, data_str);
		if (r > 0)
		{
			return 1;
		}

		// WIF
		r = privkey_from_wif(key, data_str);
		if (r > 0)
		{
			return 1;
		}

		// String
		r = privkey_from_str(key, data_str);
		if (r > 0)
		{
			return 1;
		}
	}

	// Raw
	r = privkey_from_raw(key, data, data_len);
	if (r > 0)
	{
		return 1;
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
