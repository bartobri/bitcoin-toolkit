#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "pubkey.h"
#include "privkey.h"
#include "point.h"
#include "crypto.h"
#include "base58check.h"
#include "bech32.h"
#include "hex.h"
#include "network.h"
#include "mem.h"
#include "assert.h"

#define ADDRESS_VERSION_BIT_MAINNET   0x00
#define ADDRESS_VERSION_BIT_TESTNET   0x6F
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
	size_t i, l;
	char *privkey_hex;
	mpz_t bignum;
	Point point;
	Point points[PUBKEY_POINTS];
	
	assert(privkey);
	assert(pubkey);

	// Private keys of zero are invalid
	if (privkey_is_zero(privkey))
	{
		return -1;
	}

	// Load private key from hex string, truncating the compression flag.
	privkey_hex = ALLOC(((PRIVKEY_LENGTH + 1) * 2) + 1);
	mpz_init(bignum);
	privkey_to_hex(privkey_hex, privkey);
	privkey_hex[PRIVKEY_LENGTH * 2] = '\0';
	mpz_set_str(bignum, privkey_hex, 16);
	free(privkey_hex);
	
	// Initalize the points
	point_init(&point);
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		point_init(points + i);
	}

	// Calculating public key
	point_set_generator(&points[0]);
	for (i = 1; i < PUBKEY_POINTS; ++i)
	{
		point_double(&points[i], points[i-1]);
		if (!point_verify(points[i]))
		{
			return -2;
		}
	}

	// Add all points corresponding to 1 bits
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		if (mpz_tstbit(bignum, i) == 1)
		{
			if (mpz_cmp_ui(point.x, 0) == 0 && mpz_cmp_ui(point.y, 0) == 0)
			{
				point_set(&point, points[i]);
			}
			else
			{
				point_add(&point, point, points[i]);
			}
			if (!point_verify(points[i]))
			{
				return -3;
			}
		}
	}
	
	// Setting compression flag
	if (privkey_is_compressed(privkey))
	{
		if (mpz_even_p(point.y))
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
	l = (mpz_sizeinbase(point.x, 2) + 7) / 8;
	mpz_export(pubkey->data + 1 + (32 - l), &i, 1, 1, 1, 0, point.x);
	if (l != i)
	{
		return -4;
	}
	if (!privkey_is_compressed(privkey))
	{
		l = (mpz_sizeinbase(point.y, 2) + 7) / 8;
		mpz_export(pubkey->data + 33 + (32 - l), &i, 1, 1, 1, 0, point.y);
		if (l != i)
		{
			return -4;
		}
	}

	// Clear all points
	mpz_clear(bignum);
	for (i = 0; i < PUBKEY_POINTS; ++i)
	{
		point_clear(points + i);
	}
	
	return 1;
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

int pubkey_is_compressed(PubKey key)
{
	assert(key);
	if (key->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || key->data[0] == PUBKEY_COMPRESSED_FLAG_ODD)
	{
		return 1;
	}
	return 0;
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

int pubkey_to_address(char *address, PubKey key)
{
	int r;
	size_t len;
	unsigned char *sha, *rmd;
	unsigned char rmd_bit[21];
	char *base58;

	assert(address);
	assert(key);

	if (pubkey_is_compressed(key))
	{
		len = PUBKEY_COMPRESSED_LENGTH + 1;
	}
	else
	{
		len = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	}

	// RMD(SHA(data))
	sha = ALLOC(32);
	r = crypto_get_sha256(sha, key->data, len);
	if (r < 0)
	{
		return -1;
	}
	rmd = ALLOC(20);
	r = crypto_get_rmd160(rmd, sha, 32);
	if (r < 0)
	{
		return -1;
	}

	// Set address version bit
	if (network_is_main())
	{
		rmd_bit[0] = ADDRESS_VERSION_BIT_MAINNET;
	}
	else if (network_is_test())
	{
		rmd_bit[0] = ADDRESS_VERSION_BIT_TESTNET;
	}
	
	// Append rmd data
	memcpy(rmd_bit + 1, rmd, 20);
	
	// Free resources
	FREE(sha);
	FREE(rmd);
	
	// Assume the base58 string will never be longer
	// than twice the input string
	base58 = ALLOC(21 * 2);
	r = base58check_encode(base58, rmd_bit, 21);
	if (r < 0)
	{
		return r;
	}

	strcpy(address, base58);

	FREE(base58);

	return 1;
}

int pubkey_to_bech32address(char *address, PubKey key)
{
	int r;
	unsigned char *sha, *rmd;

	assert(address);
	assert(key);

	if (!pubkey_is_compressed(key))
	{
		return -1;
	}

	// RMD(SHA(data))
	sha = ALLOC(32);
	r = crypto_get_sha256(sha, key->data, PUBKEY_COMPRESSED_LENGTH + 1);
	if (r < 0)
	{
		return -1;
	}
	rmd = ALLOC(20);
	r = crypto_get_rmd160(rmd, sha, 32);
	if (r < 0)
	{
		return -1;
	}

	r = bech32_get_address(address, rmd, 20);
	if (r < 0)
	{
		return -1;
	}
	
	// Free resources
	FREE(sha);
	FREE(rmd);

	return 1;
}

void pubkey_free(PubKey key)
{
	assert(key);
	FREE(key);
}

size_t pubkey_sizeof(void)
{
	return sizeof(struct PubKey);
}

