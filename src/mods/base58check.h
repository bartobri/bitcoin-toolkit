#ifndef BASE58CHECK_H
#define BASE58CHECK_H 1

char *base58check_encode(unsigned char *, size_t);
unsigned char *base58check_decode(char *, size_t, size_t *);

#endif
