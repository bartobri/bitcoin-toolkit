#include <gmp.h>
#include "privkey.h"
#include "random.h"

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
	
	return k;
}

PrivKeyComp privkey_compress(PrivKey p) {
	int i;
	PrivKeyComp r;
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		r.data[i] = p.data[i];
	}
	r.data[i] = 0x01;
	
	return r;
}

PrivKeyComp privkey_new_compressed(void) {
	PrivKey k;
	PrivKeyComp c;
	
	k = privkey_new();
	c = privkey_compress(k);
	
	return c;
}
