/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef ADDRESSDB_H
#define ADDRESSDB_H 1

#include <stdint.h>
#include <stdbool.h>

int addressdb_open(char *, bool);
void addressdb_close(void);
int addressdb_get(uint64_t *, char *);
int addressdb_put(char *, uint64_t);

#endif