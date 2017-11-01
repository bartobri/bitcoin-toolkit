#include "stddef.h"
#include "privkey.h"
#include "base58check.h"

#define MAINNET_PREFIX      0x80
#define TESTNET_PREFIX      0xEF

char *wif_get(PrivKey k) {
	int i, l;
	unsigned char kp[PRIVKEY_LENGTH + 2];
	
	if (privkey_is_compressed(k)) {
		l = PRIVKEY_LENGTH + 2;
	} else {
		l = PRIVKEY_LENGTH + 1;
	}

	kp[0] = MAINNET_PREFIX;
	for (i = 0; i < l-1; ++i) {
		kp[i+1] = k->data[i];
	}
	
	return base58check_encode(kp, l);
}
