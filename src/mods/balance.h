/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BALANCE_H
#define BALANCE_H 1

#include <stdint.h>
#include <stdbool.h>

int balance_open(char *, bool);
void balance_close(void);
int balance_get(uint64_t *, char *);
int balance_put(char *, uint64_t);
int balance_delete(char *);
int balance_batch_put(char *, uint64_t);
int balance_batch_delete(char *address);
int balance_batch_write(void);

int balance_get_record_count(size_t *);

#endif