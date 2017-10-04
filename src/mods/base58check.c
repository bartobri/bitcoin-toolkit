#include <stdlib.h>
#include "base58.h"
#include "crypto.h"

#define CHECKSUM_LENGTH 4

char *base58check_encode(unsigned char *s, size_t l) {
	size_t i;
	unsigned char *scheck;
	unsigned char *sha;
	char *r;
	
	// Get memory for the checksummed string
	if ((scheck = malloc(l + CHECKSUM_LENGTH)) == NULL) {
		// TODO: handle error
	}
	
	// Copy s to scheck
	for (i = 0; i < l; ++i) {
		scheck[i] = s[i];
	}
	
	// SHA256(SHA256(s))
	sha = crypto_get_sha256(crypto_get_sha256(s, l), 32);
	
	// First 4 bytes of checksum as last 4 bytes of return value
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		scheck[l+i] = sha[i];
	}
	
	// Encode checksumed string
	r = base58_encode(scheck, l + CHECKSUM_LENGTH);
	
	// Free memory
	free(scheck);
	//free(sha);  // TODO - why can't i free this?
	
	return r;
}
