#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "base32.h"
#include "mem.h"
#include "assert.h"

static char *code_string = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

void base32_encode(char *output, unsigned char *data, size_t data_len) {
	size_t i, output_len;

	assert(output);
	assert(data);
	assert(data_len);

	base32_encode_raw((unsigned char*)output, &output_len, data, data_len);

	for (i = 0; i < output_len; ++i) {
		output[i] = base32_get_char((uint8_t)output[i]);
	}
	output[i] = '\0';
}

void base32_encode_raw(unsigned char *output, size_t *output_len, unsigned char *data, size_t data_len) {
	size_t i, j, k;
	unsigned char *output_head = output;

	assert(output);
	assert(data);
	assert(data_len);

	*output = 0;
	for (i = 0, k = 0; i < data_len; ++i) {
		for (j = 0; j < 8; ++j) {
			*output <<= 1;
			*output += ((data[i] << j) & 0x80) >> 7;
			if ((++k % 5) == 0) {
				output++;
				*output = 0;
			}
		}
	}
	*output_len = ++k / 5;
	output = output_head;
}

char base32_get_char(uint8_t c) {
	assert(c < strlen(code_string));
	return code_string[c];
}

int base32_get_raw(char c) {
	size_t i;

	assert(c);

	for (i = 0; i < strlen(code_string); ++i) {
		if (c == code_string[i]) {
			return i;
		}
	}

	return -1;
}