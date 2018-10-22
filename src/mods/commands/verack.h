/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef VERACK_H
#define VERACK_H 1

#include <stddef.h>

#define VERACK_COMMAND "verack"

typedef struct Verack *Verack;

int verack_new(Verack);
size_t verack_sizeof(void);

#endif
