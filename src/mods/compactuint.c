#include <stdlib.h>
#include <stdint.h>

uint64_t compactuint_get_value(unsigned char *raw, size_t *c) {
	size_t i, l;
	uint64_t r;

	if (*raw <= 0xfc) {
		r = *raw;
		*c = 1;
	} else {
		if (*raw == 0xfd) {
			l = 2;
		} else if (*raw == 0xfe) {
			l = 4;
		} else if (*raw == 0xff) {
			l = 8;
		}
		
		++raw;

		for (r = 0, i = 0; i < l; ++i, ++raw) {
			r <<= 8;
			r += *raw;
		}
		
		*c = l + 1;
	}
	
	return r;
}

