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
#include <openssl/evp.h>
#ifdef EVP_H_MISSING
#  include "crypto/rmd160.h"
#  include "crypto/sha256.h"
#else
#  include <openssl/provider.h>
#endif
#include "crypto.h"
#include "error.h"

int crypto_get_sha256(unsigned char *output, unsigned char *input, size_t input_len)
{
	assert(output);
	assert(input);

# ifdef EVP_H_MISSING
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, input, input_len);
	SHA256_Final(output, &sha256);
# else
	EVP_MD_CTX *mdctx;
	unsigned int output_len;
	mdctx = EVP_MD_CTX_new();
	ERROR_CHECK_NULL(mdctx, "Memory allocation error.");
	EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
	EVP_DigestUpdate(mdctx, input, input_len);
	EVP_DigestFinal_ex(mdctx, output, &output_len);
	EVP_MD_CTX_free(mdctx);
# endif
	
	return 1;
}

int crypto_get_rmd160(unsigned char *output, unsigned char *input, size_t input_len)
{
	assert(output);
	assert(input);
	assert(input_len);

# ifdef EVP_H_MISSING
	RIPEMD160_CTX rmd160;
	RIPEMD160_Init(&rmd160);
	RIPEMD160_Update(&rmd160, input, input_len);
	RIPEMD160_Final(output, &rmd160);
# else
	int r;
	EVP_MD_CTX *mdctx;
	unsigned int output_len;
#   ifndef PROVIDER_H_MISSING
		OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "legacy");
#   endif
	mdctx = EVP_MD_CTX_new();
	ERROR_CHECK_NULL(mdctx, "Memory allocation error.");
	r = EVP_DigestInit_ex(mdctx, EVP_ripemd160(), NULL);
	ERROR_CHECK_FALSE(r, "Could not initialize rmd digest.");
	EVP_DigestUpdate(mdctx, input, input_len);
	EVP_DigestFinal_ex(mdctx, output, &output_len);
	EVP_MD_CTX_free(mdctx);
#   ifndef PROVIDER_H_MISSING
		OSSL_PROVIDER_unload(prov);
#   endif
# endif

	return 1;
}

int crypto_get_checksum(uint32_t *output, unsigned char *data, size_t len)
{
	int r;
	unsigned char *sha1, *sha2;

	assert(output);
	assert(data);

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
