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

struct PubKey{
	unsigned char data[PUBKEY_UNCOMPRESSED_LENGTH + 1];
};

int pubkey_get(PubKey pubkey, PrivKey privkey) {
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

PubKey pubkey_compress(PubKey k) {
	mpz_t y;
	size_t point_length = 32;
	
	assert(k);
	
	if (k->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k->data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		return k;
	}

	mpz_init(y);

	mpz_import(y, point_length, 1, 1, 1, 0, k->data + 1 + point_length);
	
	if (mpz_even_p(y)) {
		k->data[0] = PUBKEY_COMPRESSED_FLAG_EVEN;
	} else {
		k->data[0] = PUBKEY_COMPRESSED_FLAG_ODD;
	}

	mpz_clear(y);
	
	return k;
}

int pubkey_is_compressed(PubKey k) {
	assert(k);
	if (k->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k->data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		return 1;
	}
	return 0;
}

char *pubkey_to_hex(PubKey k) {
	int i, l;
	char *r;
	
	assert(k);
	
	if (k->data[0] == PUBKEY_UNCOMPRESSED_FLAG) {
		l = (PUBKEY_UNCOMPRESSED_LENGTH * 2) + 2;
	} else if (k->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k->data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		l = (PUBKEY_COMPRESSED_LENGTH * 2) + 2;
	} else {
		return NULL;
	}

	r = ALLOC(l + 1);
	if (r == NULL) {
		return NULL;
	}

	memset(r, 0, l + 1);
	
	for (i = 0; i < l/2; ++i) {
		sprintf(r + (i * 2), "%02x", k->data[i]);
	}
	
	return r;
}

unsigned char *pubkey_to_raw(PubKey k, size_t *rl) {
	int i, l;
	char *s;
	unsigned char *r;
	
	s = pubkey_to_hex(k);
	
	l = strlen(s);
	
	r = ALLOC(l/2);
	
	for (i = 0; i < l; i += 2) {
		r[i/2] = hex_to_dec(s[i], s[i+1]);
	}
	
	*rl = l/2;
	
	return r;
}

char *pubkey_to_address(PubKey k) {
	size_t l;
	unsigned char *sha, *rmd;
	unsigned char r[21];

	if (pubkey_is_compressed(k)) {
		l = PUBKEY_COMPRESSED_LENGTH + 1;
	} else {
		l = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	}

	// RMD(SHA(data))
	sha = crypto_get_sha256(k->data, l);
	rmd = crypto_get_rmd160(sha, 32);

	// Set address version bit
	if (network_is_main()) {
		r[0] = ADDRESS_VERSION_BIT_MAINNET;
	} else if (network_is_test()) {
		r[0] = ADDRESS_VERSION_BIT_TESTNET;
	}
	
	// Append rmd data
	memcpy(r + 1, rmd, 20);
	
	// Free resources
	FREE(sha);
	FREE(rmd);
	
	return base58check_encode(r, 21);
}

char *pubkey_to_bech32address(PubKey k) {
	unsigned char *sha, *rmd;
	char *output;

	assert(pubkey_is_compressed(k));

	// RMD(SHA(data))
	sha = crypto_get_sha256(k->data, PUBKEY_COMPRESSED_LENGTH + 1);
	rmd = crypto_get_rmd160(sha, 32);

	output = ALLOC(100);
	bech32_get_address(output, rmd, 20);
	
	// Free resources
	FREE(sha);
	FREE(rmd);

	return output;
}

void pubkey_free(PubKey k) {
	FREE(k);
}

size_t pubkey_sizeof(void)
{
	return sizeof(struct PubKey);
}

