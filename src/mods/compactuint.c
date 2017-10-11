#include <stdlib.h>
#include <stdint.h>
#include "hex.h"

uint64_t compactuint_get_value(char *hex, size_t *c) {
	size_t l;
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
		
		r = hex_to_dec_substr(0, hex + 2, l);
		
		*c = l + 2;
	}
	
	return r;
}

