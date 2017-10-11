#include <stdlib.h>
#include <stdint.h>
#include "hex.h"

uint64_t compactuint_get_value(char *hex, size_t *c) {
	size_t i, l;
	char t[sizeof(uint64_t) * 2 + 1];
	uint64_t r;

	if (hex_to_dec(hex[0], hex[1]) <= 0xfc) {
		r = hex_to_dec(hex[0], hex[1]);
		*c = 2;
	} else {
		if (hex_to_dec(hex[0], hex[1]) == 0xfd) {
			l = 2 * 2;
		} else if (hex_to_dec(hex[0], hex[1]) == 0xfe) {
			l = 4 * 2;
		} else if (hex_to_dec(hex[0], hex[1]) == 0xff) {
			l = 8 * 2;
		}

		for (i = 0, hex += 2; i < l; ++i, ++hex) {
			t[i] = hex[0];
		}
		t[i] = '\0';

		r = (uint64_t)strtoull(t, NULL, 16);
		
		*c = l + 2;
	}
	
	return r;
}

