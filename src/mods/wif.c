#include <string.h>
#include <stddef.h>
#include "privkey.h"
#include "base58check.h"
#include "mem.h"
#include "hex.h"

#define MAINNET_PREFIX      0x80
#define TESTNET_PREFIX      0xEF

char *wif_get(PrivKey k) {
	int i, j, l;
	char *r;
	char *privkeyhex;
	unsigned char *privkeyraw;
	
	privkeyhex = privkey_to_hex(k);

	l = strlen(privkeyhex) / 2 + 1;
	
	privkeyraw = ALLOC(l);

	privkeyraw[0] = MAINNET_PREFIX;
	for (i = 1, j = 0; i < l; ++i, j += 2) {
		privkeyraw[i] = hex_to_dec(privkeyhex[j], privkeyhex[j+1]);
	}
	
	r = base58check_encode(privkeyraw, l);
	
	FREE(privkeyhex);
	FREE(privkeyraw);
	
	return r;
}
