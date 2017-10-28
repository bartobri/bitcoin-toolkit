#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/wif.h"

int main(void) {
	size_t i;
	int c;
	PrivKey key;

	memset(key.data, 0, PRIVKEY_LENGTH + 1);

	// Geting input
	for (i = 0; (c = getchar()) != EOF; i = (i + 1) % PRIVKEY_LENGTH) {	
		key.data[i] = (unsigned char)(((int)key.data[i] + c) % UCHAR_MAX);
	}
	
	printf("WIF: %s\n", wif_get(key));
	printf("Address: %s\n", address_get(pubkey_get(key)));
	
	return 0;
}
