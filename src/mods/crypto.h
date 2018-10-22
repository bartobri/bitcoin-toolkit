/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef CRYPTO_H
#define CRYPTO_H 1

#include <stdint.h>

int crypto_get_sha256(unsigned char *, unsigned char *, size_t);
int crypto_get_rmd160(unsigned char *, unsigned char *, size_t);
int crypto_get_checksum(uint32_t *, unsigned char *, size_t);

#endif
