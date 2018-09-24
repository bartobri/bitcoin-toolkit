#ifndef HEX_H
#define HEX_H 1

#include <stddef.h>
#include <stdint.h>

int hex_to_dec(char, char);
uint64_t       hex_to_dec_substr(size_t, char *, size_t);
unsigned char *hex_str_to_uc(char *);
int            hex_ischar(char);

#endif
