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
	PrivKeyComp key;

	memset(key.data, 0, PRIVKEY_COMP_LENGTH);

	// Geting input
	for (i = 0; (c = getchar()) != EOF; i = (i + 1) % PRIVKEY_COMP_LENGTH) {	
		key.data[i] = (unsigned char)(((int)key.data[i] + c) % UCHAR_MAX);
	}
	
	printf("WIF: %s\n", wif_get_compressed(key));
	printf("Address: %s\n", address_get_compressed(pubkey_get_compressed(key)));
	
	return 0;
}
