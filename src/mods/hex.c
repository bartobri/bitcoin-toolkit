#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "assert.h"

unsigned char hex_to_dec(char l, char r) {
	unsigned char ret;

	assert((l >= 'A' && l <= 'F') || (l >= '0' && l <= '9') || (l >= 'a' && l <= 'z'));
	assert((r >= 'A' && r <= 'F') || (r >= '0' && r <= '9') || (r >= 'a' && r <= 'z'));

	// force lowercase
	if (l >= 'A' && l <= 'F') {
		l -= 55;
	}
	if (r >= 'A' && r <= 'F') {
		r -= 55;
	}
	
	// Deal with numbers
	if (l >= '0' && l <= '9') {
		l -= 48;
	}
	if (r >= '0' && r <= '9') {
		r -= 48;
	}
	
	// Deal with letters
	if (l >= 'a' && l <= 'z') {
		l -= 87;
	}
	if (r >= 'a' && r <= 'z') {
		r -= 87;
	}
	
	ret = (l << 4) + r;
	
	return ret;
}

uint64_t hex_to_dec_substr(size_t offset, char *str, size_t len) {
	size_t i;
	char *t;
	unsigned long long r;
	
	if (offset <= strlen(str)) {
		str += offset;
	} else {
		// Handle error here, offset larger than string len
	}
	
	if (len > strlen(str)) {
		// Handle error here, len larger than available string len
	}
	
	if ((t = malloc(len + 1)) == NULL) {
		// Handle memory error here
	}

	for (i = 0; i < len; ++i, ++str) {
		t[i] = *str;
	}
	t[i] = '\0';
	
	errno = 0;

	r = strtoull(t, NULL, 16);
	
	if (errno == ERANGE) {
		// Handle overflow error here
	}
	
	free(t);
	
	return (uint64_t)r;
}

// TODO - validate string is fully hexidecimal and even numbered
unsigned char *hex_str_to_uc(char *hex) {
	size_t i, l;
	unsigned char *raw;
	
	l = strlen(hex);
	
	if ((raw = malloc(l / 2)) == NULL) {
		// TODO - handle memory error
	}
	
	for (i = 0; i < l; ++i, hex += 2) {
		raw[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	return raw;
}

