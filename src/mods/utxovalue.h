/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef UTXOVALUE_H
#define UTXOVALUE_H 1

#include <stdint.h>

typedef struct UTXOValue *UTXOValue;
struct UTXOValue
{
	uint64_t       height;
	uint64_t       amount;
	uint64_t       n_size;
	size_t         script_len;
	unsigned char *script;
};

int utxovalue_from_raw(UTXOValue, unsigned char *);
void utxovalue_free(UTXOValue);

#endif
