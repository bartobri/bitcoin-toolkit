/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef SCRIPT_H
#define SCRIPT_H 1

#include <stdint.h>

const char *script_get_word(uint8_t);
char *script_from_raw(unsigned char *, size_t);
int script_get_output_address(char *, unsigned char*, uint64_t, uint32_t);

#endif
