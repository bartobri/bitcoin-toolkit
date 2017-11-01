#include <stddef.h>
#include <string.h>
#include "pubkey.h"
#include "crypto.h"
#include "base58check.h"
#include "mem.h"

#define ADDRESS_VERSION_BIT 0x00

char *address_get(PubKey k) {
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
	r[0] = ADDRESS_VERSION_BIT;
	
	// Append rmd data
	memcpy(r + 1, rmd, 20);
	
	// Free resources
	FREE(sha);
	FREE(rmd);
	
	return base58check_encode(r, 21);
}
