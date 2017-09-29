#include <stdlib.h>
#include <gcrypt.h>
#include "base58_check/base58.h"

#define CHECKSUM_LENGTH 4

char *base58_check_encode(unsigned char *s, size_t l) {
	size_t i;
	gcry_md_hd_t h1, h2;
	unsigned char *scheck;
	unsigned char *sha1;
	unsigned char *sha2;
	char *r;
	
	// Initialize my gcrypt variables
	gcry_md_open(&h1, GCRY_MD_SHA256, 0);
	gcry_md_open(&h2, GCRY_MD_SHA256, 0);
	
	// Get memory for the checksummed string
	if ((scheck = malloc(l + CHECKSUM_LENGTH)) == NULL) {
		// TODO: handle error
	}

	// SHA256(SHA256(s))
	for (i = 0; i < l; ++i) {
		gcry_md_putc(h1, s[i]);
	}
	sha1 = gcry_md_read(h1, 0);

	for (i = 0; i < 32; ++i) {
		gcry_md_putc(h2, sha1[i]);
	}
	sha2 = gcry_md_read(h2, 0);
	
	// First 4 bytes of checksum as last 4 bytes of return value
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		scheck[l+i] = sha2[i];
	}
	
	// Encode checksumed string
	r = base58_encode(scheck, l + CHECKSUM_LENGTH);
	
	// Free memory
	free(scheck);
	free(sha1);
	free(sha2);
	
	// Free gcrypt variables
	gcry_md_close(h1);
	gcry_md_close(h2);
	
	return r;
}
