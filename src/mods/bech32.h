/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BECH32_H
#define BECH32_H 1

#include <stddef.h>

int bech32_get_address(char *, unsigned char *, size_t, int);

#endif