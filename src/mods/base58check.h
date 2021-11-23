/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BASE58CHECK_H
#define BASE58CHECK_H 1

#define BASE58CHECK_TYPE_NA              0
#define BASE58CHECK_TYPE_ADDRESS_MAINNET 1

int base58check_encode(char *, unsigned char *, size_t);
int base58check_decode(unsigned char *, char *, int);

#endif
