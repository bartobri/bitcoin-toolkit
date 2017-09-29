#include <stdio.h>
#include "bitcoin/privkey.h"
#include "bitcoin/pubkey.h"
#include "bitcoin/crypto.h"
#include "bitcoin/base58_check.h"

#define ADDRESS_VERSION_BIT 0x00

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

void bitcoin_print_privkey_address(void) {
	size_t i;
	PrivKey k;
	PubKey p;
	unsigned char *d;
	
	// Get keys
	k = privkey_new();
	p = pubkey_get(k);

	// Crypto
	d = crypto_get_rmd160(crypto_get_sha256(p.data, PUBKEY_LENGTH), 32);

	// Prepend address version bit
	for (i = 20; i > 0; --i) {
		d[i] = d[i-1];
	}
	d[0] = ADDRESS_VERSION_BIT;

	// Print
	printf("Private: ");
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
	printf("\n");
	
	printf("Address: %s\n", base58_check_encode(d, 21));
}
