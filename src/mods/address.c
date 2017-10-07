#include <stddef.h>
#include "pubkey.h"
#include "crypto.h"
#include "base58check.h"

#define ADDRESS_VERSION_BIT 0x00

char *address_get(PubKey k) {
	size_t i, l;
	unsigned char *d;
	unsigned char d2[21];

	if (pubkey_is_compressed(k)) {
		l = PUBKEY_COMPRESSED_LENGTH + 1;
	} else {
		l = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	}

	d = crypto_get_rmd160(crypto_get_sha256(k.data, l), 32);

	// Prepend address version bit
	d2[0] = ADDRESS_VERSION_BIT;
	for (i = 0; i < 20; ++i) {
		d2[i+1] = d[i];
	}
	
	return base58check_encode(d2, 21);
}
