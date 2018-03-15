#include <stddef.h>
#include <string.h>
#include "base32.h"
#include "mem.h"
#include "assert.h"

static char *code_string = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

char *base32_encode(unsigned char *data, size_t data_len) {
	size_t i, j, k;
	char *rp, *rh;

	// TODO - need asserts

	rp = rh = ALLOC(data_len * 2);

	memset(rp, 0, data_len * 2);

	for (i = 0, k = 0; i < data_len; ++i) {
		for (j = 0; j < 8; ++j) {
			*rp <<= 1;
			*rp += ((data[i] << j) & 0x80) >> 7;
			if ((++k % 5) == 0) {
				*rp = code_string[(int)*rp];
				rp++;
			}
		}
	}

	if ((k % 5) != 0) {
		*rp = code_string[(int)*rp];
		rp++;
	}

	*rp = '\0';

	return rh;
}

char base32_get_char(size_t c) {
	assert(c < strlen(code_string));
	return code_string[c];
}

int base32_get_raw(char c) {
	size_t i;

	for (i = 0; i < strlen(code_string); ++i) {
		if (c == code_string[i]) {
			return i;
		}
	}

	return -1;
}