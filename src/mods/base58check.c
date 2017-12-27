#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "base58.h"
#include "crypto.h"
#include "mem.h"
#include "assert.h"

#define CHECKSUM_LENGTH 4

char *base58check_encode(unsigned char *s, size_t l) {
	unsigned char *scheck;
	unsigned char *sha1, *sha2;
	char *r;
	
	assert(s);
	assert(l);
	
	// Get memory for the checksummed string
	scheck = ALLOC(l + CHECKSUM_LENGTH);
	
	// Copy s to scheck
	memcpy(scheck, s, l);
	
	// SHA256(SHA256(s))
	sha1 = crypto_get_sha256(s, l);
	sha2 = crypto_get_sha256(sha1, 32);
	
	// First 4 bytes of checksum as last 4 bytes of scheck
	memcpy(scheck + l, sha2, CHECKSUM_LENGTH);
	
	// Encode checksumed string
	r = base58_encode(scheck, l + CHECKSUM_LENGTH);
	
	// Free memory
	FREE(scheck);
	FREE(sha1);
	FREE(sha2);
	
	return r;
}

unsigned char *base58check_decode(char *s, size_t l, size_t *rl) {
	unsigned char *r;
	size_t i, b58l;
	uint32_t checksum1 = 0, checksum2 = 0;
	
	assert(s);
	assert(l);
	
	r = base58_decode(s, l, &b58l);
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		checksum1 <<= 8;
		checksum1 += r[b58l-CHECKSUM_LENGTH+i];
	}

	checksum2 = crypto_get_checksum(r, b58l - CHECKSUM_LENGTH);
	
	assert(checksum1 == checksum2);
	
	*rl = b58l - CHECKSUM_LENGTH;

	return r;
}

