#include "network.h"

#define MAINNET 1;
#define TESTNET 2;

static int network_type = MAINNET;

void network_set_main(void) {
	network_type = MAINNET;
}

void network_set_test(void) {
	network_type = TESTNET;
}

int network_is_main(void) {
	return network_type == MAINNET;
}

int network_is_test(void) {
	return network_type == TESTNET;
}