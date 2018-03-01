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
	uint32_t i, checksum;
	char *r;
	
	assert(s);
	assert(l);
	
	// Get memory for the checksummed string
	scheck = ALLOC(l + CHECKSUM_LENGTH);
	
	// Copy s to scheck
	memcpy(scheck, s, l);
	
	// get checksum
	checksum = crypto_get_checksum(s, l);
	
	// Copy bytes of checksum (big endian) as last 4 bytes of scheck
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		scheck[l+i] = ((checksum >> (24-i*8)) & 0x000000FF);
	}
	
	// Encode checksumed string
	r = base58_encode(scheck, l + CHECKSUM_LENGTH);
	
	// Free memory
	FREE(scheck);
	
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

int base58check_valid_checksum(char *s, size_t l) {
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
	
	return checksum1 == checksum2;
}