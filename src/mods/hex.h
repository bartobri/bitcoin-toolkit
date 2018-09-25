#ifndef HEX_H
#define HEX_H 1

#include <stddef.h>
#include <stdint.h>

int hex_to_dec(char, char);
int hex_str_to_raw(unsigned char *, char *);
int hex_ischar(char);

#endif
