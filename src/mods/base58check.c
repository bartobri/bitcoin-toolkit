/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "base58.h"
#include "crypto.h"
#include "error.h"
#include "base58check.h"

#define CHECKSUM_LENGTH 4

int base58check_encode(char *output, unsigned char *input, size_t input_len) {
	int i, r;
	uint32_t checksum;
	unsigned char *input_check;
	
	assert(output);
	assert(input);
	assert(input_len);
	
	input_check = malloc(input_len + CHECKSUM_LENGTH);
	if (input_check == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	
	memcpy(input_check, input, input_len);
	
	r = crypto_get_checksum(&checksum, input, input_len);
	if (r < 0)
	{
		error_log("Could not generate checksum from input.");
		return -1;
	}
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i)
	{
		input_check[input_len+i] = ((checksum >> (24-i*8)) & 0x000000FF);
	}
	
	r = base58_encode(output, input_check, input_len + CHECKSUM_LENGTH);
	if (r < 0)
	{
		error_log("Could not encode input to base58.");
		return -1;
	}
	
	free(input_check);
	
	return 1;
}

int base58check_decode(unsigned char *output, char *input, int type) {
	int i, r, len;
	uint32_t checksum1 = 0, checksum2 = 0;
	
	assert(input);
	assert(output);

	r = base58_decode(output, input);
	if (r < 0)
	{
		error_log("Could not decode input from base58.");
		return -1;
	}

	if (type == BASE58CHECK_TYPE_ADDRESS_MAINNET)
	{
		// With mainnet addresses we have to pad the output
		// with leading zeros so that we have 25 bytes in total.
		// Leading zeros are lost in the decoding algorithm and
		// only mainnet addresses have this requirement because
		// the mainnet flag itself is a zero.
		while (r < 25)
		{
			for (i = 0; i < 24; i++)
			{
				output[24 - i] = output[24 - 1 - i];
				output[24 - 1 - i] = 0x00;
			}
			r++;
		}
	}

	len = r;
	len -= CHECKSUM_LENGTH;

	if (len <= 0)
	{
		error_log("Length of decoded data (%i) is too short to contain both data and checksum.", r);
		return -1;
	}
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		checksum1 <<= 8;
		checksum1 += output[len+i];
	}

	r = crypto_get_checksum(&checksum2, output, len);
	if (r < 0)
	{
		error_log("Could not generate checksum from decoded data.");
		return -1;
	}
	
	if (checksum1 != checksum2)
	{
		error_log("Input contains invalid checksum.");
		return -1;
	}

	return len;
}