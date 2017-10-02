#include <stdio.h>
#include "bitcoin/privkey.h"
#include "bitcoin/pubkey.h"
#include "bitcoin/crypto.h"
#include "bitcoin/base58check.h"

#define ADDRESS_VERSION_BIT 0x00
#define MAINNET_PREFIX      0x80
#define TESTNET_PREFIX      0xEF

void bitcoin_print_privkey(void) {
	int i;
	PrivKey k = privkey_new();
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
}

void bitcoin_print_privkey_compressed(void) {
	int i;
	PrivKeyComp c;

	c = privkey_new_compressed();
	
	printf("Compressed: ");
	for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
		printf("%02x", c.data[i]);
	}
	printf("\n");
}

void bitcoin_print_privkey_wif(void) {
	int i;
	PrivKey k;
	unsigned char kp[PRIVKEY_LENGTH + 1];
	char *wif;
	
	k = privkey_new();

	kp[0] = MAINNET_PREFIX;
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		kp[i+1] = k.data[i];
	}
	
	wif = base58check_encode(kp, PRIVKEY_LENGTH + 1);
	
	printf("Pivate: ");
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
	printf("\n");
	
	printf("WIF: %s\n", wif);
}

void bitcoin_print_privkey_compressed_wif(void) {
	int i;
	PrivKey k;
	PrivKeyComp c;
	unsigned char cp[PRIVKEY_COMP_LENGTH + 1];
	char *wif;
	
	k = privkey_new();
	c = privkey_compress(k);

	cp[0] = MAINNET_PREFIX;
	for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
		cp[i+1] = c.data[i];
	}
	
	wif = base58check_encode(cp, PRIVKEY_COMP_LENGTH + 1);
	
	printf("Pivate: ");
	for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
		printf("%02x", c.data[i]);
	}
	printf("\n");
	
	printf("WIF: %s\n", wif);
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

void bitcoin_print_privkey_compressed_pubkey_compressed(void) {
	int i;
	PrivKey k;
	PrivKeyComp kc;
	PubKey p;
	PubKeyComp pc;
	
	k = privkey_new();
	kc = privkey_compress(k);
	
	for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
		printf("%02x", kc.data[i]);
	}
	printf("\n");
	
	p = pubkey_get(k);
	pc = pubkey_compress(p);
	
	for (i = 0; i < PUBKEY_COMP_LENGTH; ++i) {
		printf("%02x", pc.data[i]);
	}
	printf("\n");
}

void bitcoin_print_privkey_address(void) {
	size_t i;
	PrivKey k;
	PubKey p;
	unsigned char *d;
	unsigned char d2[21];
	
	// Get keys
	k = privkey_new();
	p = pubkey_get(k);

	// Crypto
	d = crypto_get_rmd160(crypto_get_sha256(p.data, PUBKEY_LENGTH), 32);

	// Prepend address version bit
	d2[0] = ADDRESS_VERSION_BIT;
	for (i = 0; i < 20; ++i) {
		d2[i+1] = d[i];
	}

	// Print
	printf("Private: ");
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
	printf("\n");
	
	printf("Address: %s\n", base58check_encode(d2, 21));
}

void bitcoin_print_privkey_compressed_address_compressed(void) {
	size_t i;
	PrivKey k;
	PrivKeyComp kc;
	PubKey p;
	PubKeyComp pc;
	
	unsigned char *d;
	unsigned char d2[21];
	
	k = privkey_new();
	kc = privkey_compress(k);
	p = pubkey_get(k);
	pc = pubkey_compress(p);

	// Crypto
	d = crypto_get_rmd160(crypto_get_sha256(pc.data, PUBKEY_COMP_LENGTH), 32);

	// Prepend address version bit
	d2[0] = ADDRESS_VERSION_BIT;
	for (i = 0; i < 20; ++i) {
		d2[i+1] = d[i];
	}

	// Print
	printf("Private Compressed: ");
	for (i = 0; i < PRIVKEY_COMP_LENGTH; ++i) {
		printf("%02x", kc.data[i]);
	}
	printf("\n");
	
	printf("Address Compressed: %s\n", base58check_encode(d2, 21));
}
