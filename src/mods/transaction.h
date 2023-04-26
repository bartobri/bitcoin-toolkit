/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef TRANSACTION_H
#define TRANSACTION_H 1

#include <stdint.h>
#include "txinput.h"
#include "txoutput.h"

#define TRANSACTION_ID_LEN 32

typedef struct Trans *Trans;
struct Trans {
	unsigned char txid[TRANSACTION_ID_LEN];
	uint32_t  version;
	uint8_t   segwit_flag;
	uint64_t  input_count;
	TXInput  *inputs;
	uint64_t  output_count;
	TXOutput *outputs;
	uint32_t  lock_time;
};

int transaction_from_raw(Trans, unsigned char *);
void transaction_free(Trans);

#endif
