/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef UTXOKEY_H
#define UTXOKEY_H 1

#include <stdint.h>

#define UTXOKEY_TX_HASH_LENGTH 32

typedef struct UTXOKey *UTXOKey;
struct UTXOKey
{
	uint8_t        type;
	unsigned char  tx_hash[UTXOKEY_TX_HASH_LENGTH];
	uint64_t       vout;
};

int utxokey_from_raw(UTXOKey, unsigned char *);
void utxokey_free(UTXOKey);

#endif
