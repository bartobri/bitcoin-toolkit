#ifndef SCRIPT_H
#define SCRIPT_H 1

#include <stdint.h>

const char *script_get_word(uint8_t);
const char *script_from_raw(unsigned char *, size_t);

#endif
