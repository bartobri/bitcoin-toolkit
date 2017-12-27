#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "mem.h"
#include "assert.h"

static char *code_string = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

char *base58_encode(unsigned char *str, size_t l) {
	int i, j;
	char t;
	char *res;
	mpz_t x, r, d;
	
	assert(str);
	assert(l);
	
	mpz_init(x);
	mpz_init(r);
	mpz_init(d);
	
	mpz_set_ui(d, 58);
	
	res = ALLOC(l*2);

	// Base58 encode
	mpz_import(x, l, 1, sizeof(str[0]), 1, 0, str);
	for (i = 0; mpz_cmp_ui(x, 0) > 0; ++i) {
		mpz_tdiv_qr(x, r, x, d);
		res[i] = code_string[mpz_get_ui(r)];
	}
	for (j = 0; str[j] == 0; ++j, ++i) {
		res[i] = code_string[0];
	}
	res[i] = '\0';
	
	// Reverse result
	for (i = 0, j = strlen(res) - 1; i < j; ++i, --j) {
		t = res[i];
		res[i] = res[j];
		res[j] = t;
	}
	
	mpz_clear(x);
	mpz_clear(r);
	mpz_clear(d);

	return res;
}

unsigned char *base58_decode(char *str, size_t l, size_t *rl) {
	size_t i, j;
	unsigned char *ret;
	mpz_t x, b;
	
	assert(str);
	assert(l);
	
	mpz_init(x);
	mpz_init(b);
	
	mpz_set_ui(b, 58);
	mpz_set_ui(x, 0);
	
	for (i = 0; i < l; ++i) {
		for (j = 0; j < strlen(code_string) && code_string[j] != str[i]; ++j)
			;
		assert(j < strlen(code_string));

		mpz_mul(x, b, x);
		mpz_add_ui(x, x, j);
	}

	ret = ALLOC((mpz_sizeinbase(x, 2) + 7) / 8);
	mpz_export(ret, rl, 1, 1, 1, 0, x);
	
	mpz_clear(x);
	mpz_clear(b);
	
	return ret;
}
