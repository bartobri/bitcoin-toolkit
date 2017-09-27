#include <stdio.h>
#include "bitcoin/privkey.h"

void bitcoin_print_privkey (void) {
	int i;
	PrivKey k = privkey_new();
	
	for (i = 0; i < PRIVKEY_LENGTH; ++i) {
		printf("%02x", k.data[i]);
	}
}
