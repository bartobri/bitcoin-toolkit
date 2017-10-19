#include <stdlib.h>
#include <stdint.h>

#define CHECK_ZERO(x)      if (x == 0) return 0;

uint64_t compactuint_get_value(unsigned char *raw, size_t l, size_t *c) {
	size_t i, j;
	uint64_t r;
	
	CHECK_ZERO(l);

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
		
		CHECK_ZERO(l);

		for (r = 0, i = 0; i < j; ++i, ++raw, --l, ++(*c)) {
			CHECK_ZERO(l);
			r <<= 8;
			r += *raw;
		}
	}
	
	return r;
}

