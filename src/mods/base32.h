#ifndef BASE32_H
#define BASE32_H 1

#include <stddef.h>

char *base32_encode(unsigned char *, size_t);
char base32_get_char(size_t);

#endif