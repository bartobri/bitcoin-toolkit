/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "hex.h"
#include "error.h"

int hex_to_dec(char l, char r)
{
	int decimal;

	assert(l);
	assert(r);

	if (!hex_ischar(l) || !hex_ischar(r))
	{
		error_log("Invalid hex character: 0x%c%c.", l, r);
		return -1;
	}

	l = tolower(l);
	r = tolower(r);

	l = isdigit(l) ? l - 48 : l - 87;
	r = isdigit(r) ? r - 48 : r - 87;
	
	decimal = (l << 4) + r;
	
	return decimal;
}

int hex_str_to_raw(unsigned char *output, char *input)
{
	int r;
	size_t i, input_len;

	assert(output);
	assert(input);
	
	input_len = strlen(input);
	
	if (input_len % 2 != 0)
	{
		error_log("Invalid hex string. Length is not even.");
		return -1;
	}
	
	for (i = 0; i < input_len; i += 2)
	{
		r = hex_to_dec(input[i], input[i + 1]);
		if (r < 0)
		{
			error_log("Could not convert hex character to decimal.");
			return -1;
		}
		*output++ = r;
	}
	
	return 1;
}

int hex_ischar(char c)
{
	assert(c);

	return (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

