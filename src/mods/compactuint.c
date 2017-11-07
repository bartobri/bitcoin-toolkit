#include <stdlib.h>
#include <stdint.h>
#include "assert.h"

uint64_t compactuint_get_value(unsigned char *raw, size_t l, size_t *c) {
	size_t i, j;
	uint64_t r;
	
	assert(raw);
	assert(l);

	if (*raw <= 0xfc) {
		r = *raw;
		*c = 1;
	} else {
		if (*raw == 0xfd) {
			j = 2;
		} else if (*raw == 0xfe) {
			j = 4;
		} else if (*raw == 0xff) {
			j = 8;
		}
		
		
		++raw;
		--l;
		*c = 1;
		
		assert(l);

		for (r = 0, i = 0; i < j; ++i, ++raw, --l, ++(*c)) {
			assert(l);
			r <<= 8;
			r += *raw;
		}
	}
	
	return r;
}

