/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef HEX_H
#define HEX_H 1

#include <stddef.h>
#include <stdint.h>

int hex_to_dec(char, char);
int hex_str_to_raw(unsigned char *, char *);
int hex_ischar(char);

#endif
