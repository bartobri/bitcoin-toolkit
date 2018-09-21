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
	
	checksum = crypto_get_checksum(input, input_len);
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i)
	{
		input_check[input_len+i] = ((checksum >> (24-i*8)) & 0x000000FF);
	}
	
	r = base58_encode(output, input_check, input_len + CHECKSUM_LENGTH);
	if (r < 0)
	{
		return r;
	}
	
	FREE(input_check);
	
	return 1;
}

int base58check_decode(unsigned char *output, char *input) {
	int i, r;
	uint32_t checksum1 = 0, checksum2 = 0;
	
	assert(input);
	assert(output);

	r = base58_decode(output, input);
	if (r < 0)
	{
		return -1;
	}

	r -= CHECKSUM_LENGTH;
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		checksum1 <<= 8;
		checksum1 += output[r+i];
	}

	checksum2 = crypto_get_checksum(output, r);
	
	if (checksum1 != checksum2)
	{
		return -2;
	}

	return r;
}

int base58check_valid_checksum(char *s, size_t l) {
	unsigned char *r;
	int i, b58l;
	uint32_t checksum1 = 0, checksum2 = 0;

	assert(s);
	assert(l);
	
	// Assume that the length of the decoded data will always
	// be less than the encoded data
	r = ALLOC(strlen(s));

	b58l = base58_decode(r, s);
	
	for (i = 0; i < CHECKSUM_LENGTH; ++i) {
		checksum1 <<= 8;
		checksum1 += r[b58l-CHECKSUM_LENGTH+i];
	}

	checksum2 = crypto_get_checksum(r, b58l - CHECKSUM_LENGTH);
	
	return checksum1 == checksum2;
}