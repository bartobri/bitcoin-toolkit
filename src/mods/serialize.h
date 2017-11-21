#ifndef SERIALIZE_H
#define SERIALIZE_H 1

#include <stdint.h>

#define SERIALIZE_ENDIAN_BIG    1
#define SERIALIZE_ENDIAN_LIT    2

unsigned char *serialize_uint32(unsigned char *, uint32_t, int);

#endif
