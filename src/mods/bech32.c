#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "bech32.h"
#include "base32.h"
#include "network.h"
#include "mem.h"
#include "assert.h"

#define BECH32_PREFIX_MAINNET         "bc"
#define BECH32_PREFIX_TESTNET         "tb"
#define BECH32_SEPARATOR              '1'
#define BECH32_VERSION_BYTE           0
#define BECH32_CHECKSUM_LENGTH        6

static void bech32_hrp_expand(unsigned char *output, size_t *output_len, char *hrp);
static uint32_t bech32_polymod(unsigned char *hrp_exp, size_t hrp_exp_len, unsigned char *data, size_t data_len);

void bech32_get_address(char *output, unsigned char *data, size_t data_len) {
	size_t i;
	char *data_b32, *hrp, checksum[BECH32_CHECKSUM_LENGTH];
	unsigned char *hrp_exp;
	size_t hrp_exp_len;
	uint32_t polymod;

	// For now I'm only supporting P2WPKH so the data length can only be 20
	assert(data_len == 20);

	// Get human readable part
	if (network_is_main()) {
		hrp = BECH32_PREFIX_MAINNET;
	} else if (network_is_test()) {
		hrp = BECH32_PREFIX_TESTNET;
	}

	// Get checksum
	hrp_exp = ALLOC(100);
	bech32_hrp_expand(hrp_exp, &hrp_exp_len, hrp);
	polymod = bech32_polymod(hrp_exp, hrp_exp_len, data, data_len) ^ 1;
	for (i = 0; i < BECH32_CHECKSUM_LENGTH; ++i) {
		//checksum[i] = (polymod >> 5 * (5 - i)) & 31;
		checksum[i] = base32_get_char((polymod >> 5 * (5 - i)) & 31);
	}

	data_b32 = base32_encode(data, data_len);

	// Assembing bech32 address
	memcpy(output, hrp, strlen(hrp));
	output[strlen(hrp)] = BECH32_SEPARATOR;
	output[strlen(hrp) + 1] = base32_get_char(BECH32_VERSION_BYTE);
	memcpy(output + strlen(hrp) + 2, data_b32, strlen(data_b32));
	memcpy(output + strlen(hrp) + 2 + strlen(data_b32), checksum, BECH32_CHECKSUM_LENGTH);
	output[strlen(hrp) + 2 + strlen(data_b32) + BECH32_CHECKSUM_LENGTH] = '\0';

	FREE(data_b32);
	FREE(hrp_exp);
	
}

static void bech32_hrp_expand(unsigned char *output, size_t *output_len, char *hrp) {
	size_t i, l;

	l = strlen(hrp);
	*output_len = l * 2 + 1;

	for (i = 0; i < l; ++i) {
		output[i] = (hrp[i] >> 5);
	}
	output[l] = 0;
	for (i = 0; i < l; ++i) {
		output[i + l + 1] = (hrp[i] & 31);
	}
}

static uint32_t bech32_polymod(unsigned char *hrp_exp, size_t hrp_exp_len, unsigned char *data, size_t data_len) {
	uint32_t gen[5] = {0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3};
	uint32_t chk = 1;
	uint8_t b;
	size_t i, j;
	unsigned char *values;
	size_t values_len;

	values_len = hrp_exp_len + 1 + data_len + 6;
	values = ALLOC(values_len);
	memcpy(values, hrp_exp, hrp_exp_len);
	values[hrp_exp_len] = BECH32_VERSION_BYTE;
	memcpy(values + hrp_exp_len + 1, data, data_len);
	values[hrp_exp_len + 1 + data_len + 0] = 0;
	values[hrp_exp_len + 1 + data_len + 1] = 0;
	values[hrp_exp_len + 1 + data_len + 2] = 0;
	values[hrp_exp_len + 1 + data_len + 3] = 0;
	values[hrp_exp_len + 1 + data_len + 4] = 0;
	values[hrp_exp_len + 1 + data_len + 5] = 0;

	for (i = 0; i < values_len; ++i) {
		b = (chk >> 25);
		chk = (chk & 0x1ffffff) << 5 ^ values[i];
		for (j = 0; j < 5; ++j) {
			chk ^= ((b >> j) & 1) ? gen[j] : 0;
		}
	}

	FREE(values);

	return chk;
}