#ifndef BASE32_H
#define BASE32_H 1

#include <stddef.h>

void base32_encode(char *, unsigned char *, size_t);
void base32_encode_raw(unsigned char *, size_t *, unsigned char *, size_t);
char base32_get_char(size_t);
int base32_get_raw(char);

#endif