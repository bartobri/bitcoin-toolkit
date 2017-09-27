#include <gcrypt.h>
#include "privkey.h"
#include "privkey/random.h"

// Private keys can not be larger than (1.158 * 10^77) - 1
#define PRIVATE_KEY_MAX "100047a327efc14f7fe934ae56989375080f11619ff7157ffffffffffffffffff"

PrivKey privkey_new(void) {
	int i;
	mpz_t cur_key, max_key;

	// Init and set max key size
	mpz_init(max_key);
	mpz_init(cur_key);
	mpz_set_str(max_key, PRIVATE_KEY_MAX, 16);
	
	// Creating private key as hex string
	while (mpz_cmp_ui(cur_key, 1) <= 0 || mpz_cmp(cur_key, max_key) >= 0) {

		for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
			key->private_key[i] = random_get_byte();
		}
		mpz_import(cur_key, PRIVATE_KEY_BYTES, 1, 1, 1, 0, key->private_key);
	}
	
	mpz_clear(max_key);
	mpz_clear(cur_key);
}
