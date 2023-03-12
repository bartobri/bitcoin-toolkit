/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef CHAINSTATE_H
#define CHAINSTATE_H 1

#include <stdint.h>
#include "utxokey.h"
#include "utxovalue.h"

int chainstate_open(char *);
int chainstate_seek_start(void);
int chainstate_get_next(UTXOKey, UTXOValue);
int chainstate_get_record_count(size_t *);
void chainstate_close(void);

#endif