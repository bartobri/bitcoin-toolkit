#include <stdio.h>
#include <string.h>
#include "modules/address.h"

int main(void) {
	unsigned char prv[PRIVATE_KEY_BYTES];
	unsigned char pub[PUBLIC_KEY_BYTES];
	int i = 0, c;
	
	memset(prv, 0, sizeof(prv));
	memset(pub, 0, sizeof(pub));

	// Geting input
	while ((c = getchar()) != EOF) {
		if (i < PRIVATE_KEY_BYTES) {
			prv[i] = (unsigned int) c;
		} else {
			break;
		}
		++i;
	}
	
	//for (i = 0; i < PRIVATE_KEY_BYTES * 2; ++i) {
	//	printf ("%c", prvhex[i]);
	//}
	//printf("\n");
	
	//for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
	//	printf ("%02x", prv[i]);
	//}
	//printf("\n");
	
	address_get(prv, pub);
	
	address_print(pub);
	
	printf("\n");
	
	return 0;
}
