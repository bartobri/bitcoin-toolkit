#include <stddef.h>
#include <string.h>
#include "bech32.h"
#include "base32.h"
#include "network.h"
#include "assert.h"

#define BECH32_PREFIX_MAINNET         "bc"
#define BECH32_PREFIX_TESTNET         "tb"
#define BECH32_SEPARATOR              '1'
#define BECH32_VERSION_BYTE           0

void bech32_get_address(char *output, unsigned char *data, size_t data_len) {
	char *data_b32, *hrp;

	// For now I'm only supporting P2WPKH so the data length can only be 20
	assert(data_len == 20);

	// Set human readable part
	if (network_is_main()) {
		hrp = BECH32_PREFIX_MAINNET;
	} else if (network_is_test()) {
		hrp = BECH32_PREFIX_TESTNET;
	}

	data_b32 = base32_encode(data, data_len);

	// Assembing bech32 address
	memcpy(output, hrp, strlen(hrp));
	output[strlen(hrp)] = BECH32_SEPARATOR;
	output[strlen(hrp) + 1] = base32_get_char(BECH32_VERSION_BYTE);
	memcpy(output + strlen(hrp) + 2, data_b32, strlen(data_b32));
	output[strlen(hrp) + 2 + strlen(data_b32)] = '\0';
}