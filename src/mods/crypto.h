#ifndef CRYPTO_H
#define CRYPTO_H 1

#include <stdint.h>

unsigned char *crypto_get_sha256(unsigned char *, size_t);
unsigned char *crypto_get_rmd160(unsigned char *, size_t);
uint32_t crypto_get_checksum(unsigned char *, size_t);

#endif
