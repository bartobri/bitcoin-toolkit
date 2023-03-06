/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "crypto.h"
#include "error.h"
#ifdef _NO_OPENSSL
#  include "crypto/rmd160.h"
#  include "crypto/sha256.h"
#else
#  include <openssl/ripemd.h>
#  include <openssl/sha.h>
#endif

int crypto_get_sha256(unsigned char *output, unsigned char *input, size_t input_len)
{
	SHA256_CTX sha256;
	
	assert(output);
	assert(input);
	assert(input_len);
	
	SHA256_Init(&sha256);

	SHA256_Update(&sha256, input, input_len);

	SHA256_Final(output, &sha256);
	
	return 1;
}

int crypto_get_rmd160(unsigned char *output, unsigned char *input, size_t input_len)
{
	RIPEMD160_CTX rmd160;
	
	assert(output);
	assert(input);
	assert(input_len);

	RIPEMD160_Init(&rmd160);

	RIPEMD160_Update(&rmd160, input, input_len);

	RIPEMD160_Final(output, &rmd160);

	return 1;
}

int crypto_get_checksum(uint32_t *output, unsigned char *data, size_t len)
{
	int r;
	unsigned char *sha1, *sha2;

	assert(output);
	assert(data);
	assert(len);

	sha1 = malloc(32);
	if (sha1 == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	sha2 = malloc(32);
	if (sha2 == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = crypto_get_sha256(sha1, data, len);
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash for input.");
		return -1;
	}
	r = crypto_get_sha256(sha2, sha1, 32);
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash for input.");
		return -1;
	}

	*output <<= 8;
	*output += sha2[0];
	*output <<= 8;
	*output += sha2[1];
	*output <<= 8;
	*output += sha2[2];
	*output <<= 8;
	*output += sha2[3];
	
	free(sha1);
	free(sha2);
	
	return 1;
}
