#ifndef BASE58_H
#define BASE58_H 1

int base58_encode(char *, unsigned char *, size_t);
int base58_decode(unsigned char *, char *);
int base58_ischar(char);

#endif
