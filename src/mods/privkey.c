#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "privkey.h"
#include "random.h"
#include "hex.h"

// Private keys can not be larger than (1.158 * 10^77) - 1
#define PRIVKEY_MAX "100047a327efc14f7fe934ae56989375080f11619ff7157ffffffffffffffffff"

PrivKey privkey_new(void) {
	int i;
	PrivKey k;
	mpz_t cur_key, max_key;

	// Init and set max key size
	mpz_init(max_key);
	mpz_init(cur_key);
	mpz_set_str(max_key, PRIVKEY_MAX, 16);
	
	// Creating private key as hex string
	while (mpz_cmp_ui(cur_key, 1) <= 0 || mpz_cmp(cur_key, max_key) >= 0) {

		for (i = 0; i < PRIVKEY_LENGTH; ++i) {
			k.data[i] = random_get();
		}
		mpz_import(cur_key, PRIVKEY_LENGTH, 1, 1, 1, 0, k.data);
	}
	
	mpz_clear(max_key);
	mpz_clear(cur_key);
	
	k.compressed = 0;
	
	return k;
}

PrivKey privkey_compress(PrivKey k) {

	k.data[PRIVKEY_COMP_LENGTH - 1] = 0x01;
	k.compressed = 1;

	return k;
}

PrivKey privkey_new_compressed(void) {
	PrivKey k;
	
	k = privkey_new();
	k = privkey_compress(k);
	
	return k;
}

char *privkey_to_hex(PrivKey k) {
	int i;
	char *r;
	
	r = malloc((PRIVKEY_COMP_LENGTH * 2) + 1);
	
	if (r == NULL) {
		return r;
	} else {
		memset(r, 0, sizeof(*r));
	}
	
	if (k.compressed) {
		for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
			sprintf(r + (i * 2), "%02x", k.data[i]);
		}
	} else {
		for (i = 0; i < PRIVKEY_LENGTH; ++i) {
			sprintf(r + (i * 2), "%02x", k.data[i]);
		}
	}
	
	return r;
}

// TODO - needs error checking for invalid hex string
PrivKey privkey_from_hex(char *hex) {
	size_t i;
	PrivKey k;
	
	if (strlen(hex) == PRIVKEY_LENGTH * 2) {
		for (i = 0; i < PRIVKEY_LENGTH * 2; i += 2) {
			k.data[i/2] = hex_to_dec(hex[i], hex[i+1]);
		}
		k.compressed = 0;
	} else if (strlen(hex) == PRIVKEY_COMP_LENGTH * 2) {
		for (i = 0; i < PRIVKEY_COMP_LENGTH * 2; i += 2) {
			k.data[i/2] = hex_to_dec(hex[i], hex[i+1]);
		}
		k.compressed = 1;
	} else {
		// TODO - handle error here
	}
	
	return k;
}
