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
unsigned char *deserialize_uint32(uint32_t *, unsigned char *, int);
unsigned char *deserialize_uchar(unsigned char *, unsigned char *, int);

#endif
