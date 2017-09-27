#include <stdio.h>
#include <string.h>
#include "modules/address.h"

int main(void) {
	unsigned char prv[PRIVATE_KEY_BYTES];
	unsigned char pub[PUBLIC_KEY_BYTES];
	char prvhex[PUBLIC_KEY_BYTES * 2];
	unsigned char hexchart[128];
	int i, j, c;
	
	memset(prv, 0, sizeof(prv));
	memset(pub, 0, sizeof(pub));
	
	for (i = 0; i < 128; ++i) {
		hexchart[i] = '\0';
	}
	hexchart['0'] = 0;
	hexchart['1'] = 1;
	hexchart['2'] = 2;
	hexchart['3'] = 3;
	hexchart['4'] = 4;
	hexchart['5'] = 5;
	hexchart['6'] = 6;
	hexchart['7'] = 7;
	hexchart['8'] = 8;
	hexchart['9'] = 9;
	hexchart['a'] = 10;
	hexchart['b'] = 11;
	hexchart['c'] = 12;
	hexchart['d'] = 13;
	hexchart['e'] = 14;
	hexchart['f'] = 15;
	
	// Geting input
	for (i = 0; (c = getchar()) != EOF && i < PRIVATE_KEY_BYTES * 2; ++i) {
		
		if (c >= 'A' && c <= 'F') {
			c -= 55;
		}
		
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
			prvhex[i] = c;
		} else {
			fprintf(stderr, "Invalid private key - Character: %c\n", c);
			return 1;
		}
	}
	if (i != PRIVATE_KEY_BYTES * 2) {
		fprintf(stderr, "Invalid private key - Length: %i\n", i);
		return 1;
	}
	
	//for (i = 0; i < PRIVATE_KEY_BYTES * 2; ++i) {
	//	printf ("%c", prvhex[i]);
	//}
	//printf("\n");
	
	for (i = 0, j = 0; i < PRIVATE_KEY_BYTES * 2; i += 2, ++j) {
		unsigned char l = hexchart[(int)prvhex[i]];
		unsigned char r = hexchart[(int)prvhex[i+1]];
		//printf("Shifting %c and %c -> ", prvhex[i], prvhex[i+1]);
		prv[j] = (l << 4) + r;
		//printf("%d\n", prv[j]);
	}
	
	//for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
	//	printf ("%02x", prv[i]);
	//}
	//printf("\n");
	
	address_get(prv, pub);
	
	address_print(pub);
	
	printf("\n");
	
	return 0;
}
