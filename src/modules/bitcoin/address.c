#include <stddef.h>
#include "pubkey.h"
#include "crypto.h"
#include "base58check.h"

#define ADDRESS_VERSION_BIT 0x00

char *address_get(PubKey p) {
	size_t i;
	unsigned char *d;
	unsigned char d2[21];

	// Crypto
	d = crypto_get_rmd160(crypto_get_sha256(p.data, PUBKEY_LENGTH), 32);

	// Prepend address version bit
	d2[0] = ADDRESS_VERSION_BIT;
	for (i = 0; i < 20; ++i) {
		d2[i+1] = d[i];
	}
	
	return base58check_encode(d2, 21);
}

char *address_get_compressed(PubKeyComp p) {
	size_t i;
	unsigned char *d;
	unsigned char d2[21];

	// Crypto
	d = crypto_get_rmd160(crypto_get_sha256(p.data, PUBKEY_COMP_LENGTH), 32);

	// Prepend address version bit
	d2[0] = ADDRESS_VERSION_BIT;
	for (i = 0; i < 20; ++i) {
		d2[i+1] = d[i];
	}
	
	return base58check_encode(d2, 21);
}
