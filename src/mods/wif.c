#include "stddef.h"
#include "privkey.h"
#include "base58check.h"

#define MAINNET_PREFIX      0x80
#define TESTNET_PREFIX      0xEF

char *wif_get(PrivKey k) {
	int i;
	unsigned char kp[PRIVKEY_LENGTH + 1];
	char *wif;

	kp[0] = MAINNET_PREFIX;
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		kp[i+1] = k.data[i];
	}
	
	wif = base58check_encode(kp, PRIVKEY_LENGTH + 1);
	
	return wif;
}

char *wif_get_compressed(PrivKeyComp k) {
	int i;
	unsigned char kp[PRIVKEY_COMP_LENGTH + 1];
	char *wif;

	kp[0] = MAINNET_PREFIX;
	for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
		kp[i+1] = k.data[i];
	}
	
	wif = base58check_encode(kp, PRIVKEY_COMP_LENGTH + 1);
	
	return wif;
}
