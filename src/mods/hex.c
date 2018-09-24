#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <limits.h>
#include <errno.h>

#include "hex.h"
#include "assert.h"
#include "mem.h"

int hex_to_dec(char l, char r)
{
	int decimal;

	assert(l);
	assert(r);

	if (!hex_ischar(l) || !hex_ischar(r))
	{
		return -1;
	}

	l = tolower(l);
	r = tolower(r);

	l = isdigit(l) ? l - 48 : l - 87;
	r = isdigit(r) ? r - 48 : r - 87;
	
	decimal = (l << 4) + r;
	
	return decimal;
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
	int r;
	size_t i, l;
	unsigned char *raw;
	
	l = strlen(hex);
	
	assert(l % 2 == 0);
	
	raw = ALLOC(l / 2);
	
	for (i = 0; i < l / 2; ++i, hex += 2) {
		r = hex_to_dec(hex[0], hex[1]);
		if (r < 0)
		{
			// return error value here
		}
		raw[i] = r;
	}
	
	return raw;
}

int hex_ischar(char c) {
	return (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

