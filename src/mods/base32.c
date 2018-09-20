#include <stddef.h>
#include "base32.h"
#include "assert.h"

#define BASE32_CODE_STRING_LENGTH 32

static char *code_string = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

int base32_encode(char *output, unsigned char *data, size_t data_len)
{
	int i, c, output_len;

	assert(output);
	assert(data);
	assert(data_len);

	output_len = base32_encode_raw((unsigned char*)output, data, data_len);

	for (i = 0; i < output_len; ++i)
	{
		c = base32_get_char((int)output[i]);
		if (c < 0)
		{
			return c;
		}
		output[i] = (char)c;
	}
	output[i] = '\0';

	return 1;
}

int base32_encode_raw(unsigned char *output, unsigned char *data, size_t data_len)
{
	size_t i, j, k;
	int r;
	unsigned char *output_head = output;

	assert(output);
	assert(data);
	assert(data_len);

	*output = 0;
	for (i = 0, k = 0; i < data_len; ++i)
	{
		for (j = 0; j < 8; ++j)
		{
			*output <<= 1;
			*output += ((data[i] << j) & 0x80) >> 7;
			if ((++k % 5) == 0)
			{
				output++;
				*output = 0;
			}
		}
	}
	r = ++k / 5;
	output = output_head;

	return r;
}

int base32_get_char(int c)
{
	if (c >= BASE32_CODE_STRING_LENGTH)
	{
		return -1;
	}
	return (int)code_string[c];
}

int base32_get_raw(char c)
{
	int i;

	assert(c);

	for (i = 0; i < BASE32_CODE_STRING_LENGTH; ++i)
	{
		if (c == code_string[i])
		{
			return i;
		}
	}

	return -1;
}