#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "base58.h"
#include "crypto.h"
#include "mem.h"
#include "assert.h"

#define CHECKSUM_LENGTH 4

int base58check_encode(char *output, unsigned char *input, size_t input_len) {
	int i, r;
	uint32_t checksum;
	unsigned char *input_check;
	
	assert(output);
	assert(input);
	assert(input_len);
	
	input_check = ALLOC(input_len + CHECKSUM_LENGTH);
	
	memcpy(input_check, input, input_len);
	
	r = crypto_get_checksum(&checksum, input, input_len);
	if (r < 0)
	{
		return -1;
	}
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i)
	{
		input_check[input_len+i] = ((checksum >> (24-i*8)) & 0x000000FF);
	}
	
	r = base58_encode(output, input_check, input_len + CHECKSUM_LENGTH);
	if (r < 0)
	{
		return -1;
	}
	
	FREE(input_check);
	
	return 1;
}

int base58check_decode(unsigned char *output, char *input) {
	int i, r, len;
	uint32_t checksum1 = 0, checksum2 = 0;
	
	assert(input);
	assert(output);

	r = base58_decode(output, input);
	if (r < 0)
	{
		return -1;
	}

	len = r;
	len -= CHECKSUM_LENGTH;
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		checksum1 <<= 8;
		checksum1 += output[len+i];
	}

	r = crypto_get_checksum(&checksum2, output, len);
	if (r < 0)
	{
		return -1;
	}
	
	if (checksum1 != checksum2)
	{
		return -1;
	}

	return len;
}

int base58check_valid_checksum(char *input) {
	int i, r;
	uint32_t checksum1 = 0, checksum2 = 0;
	unsigned char *decoded;

	assert(input);
	
	// Assume that the length of the decoded data will always
	// be less than the encoded data
	decoded = ALLOC(strlen(input));

	r = base58_decode(decoded, input);
	if (r < 0)
	{
		return -1;
	}

	r -= CHECKSUM_LENGTH;
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		checksum1 <<= 8;
		checksum1 += decoded[r + i];
	}

	r = crypto_get_checksum(&checksum2, decoded, r);
	if (r < 0)
	{
		return -1;
	}

	FREE(decoded);
	
	return checksum1 == checksum2;
}