/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef SERIALIZE_H
#define SERIALIZE_H 1

#include <stdint.h>

#define SERIALIZE_ENDIAN_BIG    1
#define SERIALIZE_ENDIAN_LIT    2

unsigned char *serialize_uint8(unsigned char *, uint8_t, int);
unsigned char *serialize_uint16(unsigned char *, uint16_t, int);
unsigned char *serialize_uint32(unsigned char *, uint32_t, int);
unsigned char *serialize_uint64(unsigned char *, uint64_t, int);
unsigned char *serialize_uchar(unsigned char *, unsigned char *, int);
unsigned char *serialize_char(unsigned char *, char *, int);
unsigned char *serialize_compuint(unsigned char *, uint64_t, int);
unsigned char *serialize_varint(unsigned char *, uint64_t);
unsigned char *deserialize_uint8(uint8_t *, unsigned char *, int);
unsigned char *deserialize_uint16(uint16_t *, unsigned char *, int);
unsigned char *deserialize_uint32(uint32_t *, unsigned char *, int);
unsigned char *deserialize_uint64(uint64_t *, unsigned char *, int);
unsigned char *deserialize_uchar(unsigned char *, unsigned char *, int, int);
unsigned char *deserialize_char(char *, unsigned char *, int);
unsigned char *deserialize_compuint(uint64_t *, unsigned char *, int);
unsigned char *deserialize_varint(uint64_t *, unsigned char *);

#endif
