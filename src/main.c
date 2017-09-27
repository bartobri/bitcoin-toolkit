#include <stdio.h>
#include "keypair.h"

int main(void) {
	KeyPair k = keypair_new();
	
	keypair_dump(k);
	
	printf("Address: ");
	keypair_print_address(&k);
	printf("\n");
	
	printf("Compressed Address: ");
	keypair_print_address_compressed(&k);
	printf("\n");
	
	printf("WIF: ");
	keypair_print_wif(&k);
	printf("\n");
	
	printf("WIF Compressed: ");
	keypair_print_wif_compressed(&k);
	printf("\n");
	
	return 0;
}
