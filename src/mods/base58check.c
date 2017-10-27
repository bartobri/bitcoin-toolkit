#include <stdlib.h>
#include <string.h>
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
