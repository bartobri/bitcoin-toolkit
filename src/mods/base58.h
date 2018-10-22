/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BASE58_H
#define BASE58_H 1

int base58_encode(char *, unsigned char *, size_t);
int base58_decode(unsigned char *, char *);
int base58_ischar(char);

#endif
