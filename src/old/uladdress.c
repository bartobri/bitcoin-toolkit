#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/wif.h"
#include "mods/mem.h"

int main(int argc, char *argv[]) {
	unsigned long n;
	char *hex;
	
	hex = ALLOC(PRIVKEY_LENGTH * 2 + 1);
	
	memset(hex, 0, PRIVKEY_LENGTH * 2 + 1);

	if (argc < 2) {
		fprintf(stderr, "Argument required\n");
		exit(EXIT_FAILURE);
	} else {
		n = strtoul(argv[1], NULL, 10);
		if (n == ULONG_MAX && errno == EINVAL) {
			fprintf(stderr, "Invalid Argument\n");
			exit(EXIT_FAILURE);
		}
	}
	
	sprintf(hex, "%064lx", n);
	
	//printf("Key: %s\n", hex);
	
	//printf("Address: %s\n", address_get(pubkey_get(privkey_from_hex(hex))));
	
	printf("%lu,%s,%s\n", n, address_get(pubkey_get(privkey_from_hex(hex))), wif_get(privkey_from_hex(hex)));

	FREE(hex);
	
	return 0;
}
