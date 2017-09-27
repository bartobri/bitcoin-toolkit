#include <stdio.h>
#include "bitcoin/privkey.h"
#include "bitcoin/pubkey.h"

void bitcoin_print_privkey(void) {
	int i;
	PrivKey k = privkey_new();
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
}

void bitcoin_print_privkey_pubkey(void) {
	int i;
	PrivKey k = privkey_new();
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
	printf("\n");
	
	PubKey p = pubkey_get(k);
	
	for (i = 0; i < PUBKEY_LENGTH; ++i) {
		printf("%02x", p.data[i]);
	}
	printf("\n");
}
