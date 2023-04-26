/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef TXOUTPUT_H
#define TXOUTPUT_H 1

#include <stdint.h>

typedef struct TXOutput *TXOutput;
struct TXOutput {
	uint64_t       amount;
	uint64_t       script_size;
	unsigned char* script_raw;
};

int txoutput_from_raw(TXOutput, unsigned char *);

#endif
