#ifndef BASE58_H
#define BASE58_H 1

char *base58_encode(unsigned char *, size_t);
unsigned char *base58_decode(char *, size_t, size_t *);
int base58_ischar(char);

#endif
