#ifndef HEX_H
#define HEX_H 1

#include <stdint.h>

unsigned char  hex_to_dec(char, char);
uint64_t       hex_to_dec_substr(size_t, char *, size_t);
unsigned char *hex_str_to_uc(char *);

#endif
