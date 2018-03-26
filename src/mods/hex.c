#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "hex.h"
#include "assert.h"
#include "mem.h"

#include <stdio.h>

unsigned char hex_to_dec(char l, char r) {
	unsigned char ret;

	assert(hex_ischar(l));
	assert(hex_ischar(r));

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
	if (l >= 'a' && l <= 'f') {
		l -= 87;
	}
	if (r >= 'a' && r <= 'f') {
		r -= 87;
	}
	
	ret = (l << 4) + r;
	
	return ret;
}

uint64_t hex_to_dec_substr(size_t offset, char *str, size_t len) {
	char *t;
	unsigned long long r;
	
	assert(offset <= strlen(str));

	str += offset;
	
	assert(len % 2 == 0);
	assert(len <= strlen(str));
	
	t = ALLOC(len + 1);
	
	memset(t, 0, len + 1);
	memcpy(t, str, len);
	
	errno = 0;

	r = strtoull(t, NULL, 16);
	
	assert(errno != ERANGE);
	assert(r <= UINT64_MAX);
	
	free(t);
	
	return (uint64_t)r;
}

unsigned char *hex_str_to_uc(char *hex) {
	size_t i, l;
	unsigned char *raw;
	
	l = strlen(hex);
	
	assert(l % 2 == 0);
	
	raw = ALLOC(l / 2);
	
	for (i = 0; i < l / 2; ++i, hex += 2) {
		raw[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	return raw;
}

int hex_ischar(char c) {
	return (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

