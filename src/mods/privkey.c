#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "privkey.h"
#include "random.h"
#include "hex.h"
#include "base58check.h"
#include "mem.h"
#include "assert.h"

#define MAINNET_PREFIX      0x80
#define TESTNET_PREFIX      0xEF

// Private keys can not be larger than (1.158 * 10^77) - 1
#define PRIVKEY_MAX                "100047a327efc14f7fe934ae56989375080f11619ff7157ffffffffffffffffff"
#define PRIVKEY_COMPRESSED_FLAG    0x01
#define PRIVKEY_UNCOMPRESSED_FLAG  0x00

struct PrivKey {
	unsigned char data[PRIVKEY_LENGTH];
	int cflag;
};

PrivKey privkey_new(void) {
	int i;
	PrivKey k;
	mpz_t cur_key, max_key;
	
	NEW(k);

	// Init and set max key size
	mpz_init(max_key);
	mpz_init(cur_key);
	mpz_set_str(max_key, PRIVKEY_MAX, 16);
	
	// Creating private key as hex string
	while (mpz_cmp_ui(cur_key, 1) <= 0 || mpz_cmp(cur_key, max_key) >= 0) {

		for (i = 0; i < PRIVKEY_LENGTH; ++i) {
			k->data[i] = random_get();
		}
		mpz_import(cur_key, PRIVKEY_LENGTH, 1, 1, 1, 0, k->data);
	}
	
	mpz_clear(max_key);
	mpz_clear(cur_key);
	
	k->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
	
	return k;
}

PrivKey privkey_compress(PrivKey k) {
	assert(k);
	k->cflag = PRIVKEY_COMPRESSED_FLAG;
	return k;
}

PrivKey privkey_uncompress(PrivKey k) {
	assert(k);
	k->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
	return k;
}

PrivKey privkey_new_compressed(void) {
	return privkey_compress(privkey_new());;
}

char *privkey_to_hex(PrivKey k) {
	int i;
	char *r;
	
	assert(k);
	
	r = ALLOC(((PRIVKEY_LENGTH + 1) * 2) + 1);
	
	memset(r, 0, ((PRIVKEY_LENGTH + 1) * 2) + 1);
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		sprintf(r + (i * 2), "%02x", k->data[i]);
	}
	if (k->cflag == PRIVKEY_COMPRESSED_FLAG) {
		sprintf(r + (i * 2), "%02x", k->cflag);
	}
	
	return r;
}

char *privkey_to_wif(PrivKey k) {
	int l;
	unsigned char p[PRIVKEY_LENGTH + 2];

	p[0] = MAINNET_PREFIX;
	memcpy(p+1, k->data, PRIVKEY_LENGTH);
	if (privkey_is_compressed(k)) {
		p[PRIVKEY_LENGTH+1] = PRIVKEY_COMPRESSED_FLAG;
		l = PRIVKEY_LENGTH + 2;
	} else {
		l = PRIVKEY_LENGTH + 1;
	}
	
	return base58check_encode(p, l);
}

PrivKey privkey_from_hex(char *hex) {
	size_t i;
	PrivKey k;
	mpz_t cur_key, max_key;
	
	// Validate hex string
	assert(hex);
	assert(strlen(hex) % 2 == 0);
	assert(strlen(hex) >= PRIVKEY_LENGTH * 2);
	for (i = 0; i < strlen(hex); ++i) {
		assert((hex[i] >= 'A' && hex[i] <= 'F') || (hex[i] >= '0' && hex[i] <= '9') || (hex[i] >= 'a' && hex[i] <= 'z'));
	}
	
	// allocate memory
	NEW(k);

	// load hex string as private key
	for (i = 0; i < PRIVKEY_LENGTH * 2; i += 2) {
		k->data[i/2] = hex_to_dec(hex[i], hex[i+1]);
	}
	if (hex[i] &&  hex[i+1] && hex_to_dec(hex[i], hex[i+1]) == PRIVKEY_COMPRESSED_FLAG) {
		k->cflag = PRIVKEY_COMPRESSED_FLAG;
	} else {
		k->cflag = PRIVKEY_UNCOMPRESSED_FLAG;
	}

	// Make sure key is not above PRIVKEY_MAX
	mpz_init(max_key);
	mpz_init(cur_key);
	mpz_set_str(max_key, PRIVKEY_MAX, 16);
	mpz_import(cur_key, PRIVKEY_LENGTH, 1, 1, 1, 0, k->data);
	assert(mpz_cmp(cur_key, max_key) < 0);
	mpz_clear(max_key);
	mpz_clear(cur_key);
	
	return k;
}

int privkey_is_compressed(PrivKey k) {
	return (k->cflag == PRIVKEY_COMPRESSED_FLAG) ? 1 : 0;
}

void privkey_free(PrivKey k) {
	FREE(k);
}
