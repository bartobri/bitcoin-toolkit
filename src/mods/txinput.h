/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef TXINPUT_H
#define TXINPUT_H 1

#include <stdint.h>

#define TXINPUT_TXHASH_LENGTH 32

typedef struct TXInput *TXInput;
struct TXInput {
	unsigned char  tx_hash[TXINPUT_TXHASH_LENGTH];
	uint32_t       index;
	uint64_t       script_size;
	unsigned char* script_raw;
	uint32_t       sequence;
	int            is_coinbase;
};

int txinput_from_raw(TXInput, unsigned char *);

#endif
