/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef TXOA_H
#define TXOA_H 1

#include <stdint.h>
#include <stdbool.h>

int txoa_open(char *, bool);
void txoa_close(void);
int txoa_get(char *, uint64_t *, unsigned char *, uint32_t);
int txoa_put(unsigned char *, uint32_t, char *);
int txoa_delete(unsigned char *, uint32_t);
int txoa_set_last_block(int);
int txoa_get_last_block(int *);
int txoa_get_record_count(size_t *);

int txoa_batch_put(unsigned char *, uint32_t, char *, uint64_t);
int txoa_batch_delete(unsigned char *, uint32_t);
int txoa_batch_write(void);

#endif