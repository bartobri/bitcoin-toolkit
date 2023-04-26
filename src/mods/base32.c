/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stddef.h>
#include <assert.h>
#include "error.h"
#include "base32.h"

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
			error_log("Could not encode input to base32.", (int)output[i]);
			return -1;
		}
		output[i] = (char)c;
	}
	output[i] = '\0';

	return 1;
}

int base32_encode_raw(unsigned char *output, unsigned char *data, size_t data_len)
{
	size_t i, j, k = 0;
	int r;

	assert(output);
	assert(data);
	assert(data_len);

	*output = 0;
	for (i = 0; i < data_len; ++i)
	{
		for (j = 0; j < 8; ++j)
		{
			*output <<= 1;
			*output += ((data[i] << j) & 0x80) >> 7;

			k++;

			if ((k % 5) == 0)
			{
				output++;
				*output = 0;
			}
		}
	}

	while (k % 5 != 0)
	{
		*output <<= 1;
		k++;
	}

	r = k / 5;

	return r;
}

int base32_get_char(int c)
{
	if (c < 0 || c >= BASE32_CODE_STRING_LENGTH)
	{
		error_log("Value %i has no corresponding base32 character.", c);
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

	error_log("Invalid base32 character: 0x%02x.", c);
	return -1;
}