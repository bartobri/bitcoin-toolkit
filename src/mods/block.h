/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BLOCK_H
#define BLOCK_H 1

#include <stdint.h>
#include "transaction.h"

#define BLOCK_PREV_LEN        32
#define BLOCK_MERKEL_ROOT_LEN 32

typedef struct Block *Block;
struct Block {
	uint32_t version;
	unsigned char prev_block[BLOCK_PREV_LEN];
	unsigned char merkel_root[BLOCK_MERKEL_ROOT_LEN];
	uint32_t timestamp;
	uint32_t bits;
	uint32_t nonce;
	uint64_t tx_count;
	Trans *transactions;
};

int block_from_raw(Block, unsigned char *);
void block_free(Block);

#endif
