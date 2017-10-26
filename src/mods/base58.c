#include <string.h>
#include <stdlib.h>
#include <gmp.h>

char *base58_encode(unsigned char *str, size_t l) {
	int i, j;
	char t;
	char *res;
	mpz_t x, r, d;
	char *code_string = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
	
	mpz_init(x);
	mpz_init(r);
	mpz_init(d);
	
	mpz_set_ui(d, 58);
	
	res = malloc(100);

	// Base58 encode
	mpz_import(x, l, 1, 1, 1, 0, str);
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

	return res;
}
