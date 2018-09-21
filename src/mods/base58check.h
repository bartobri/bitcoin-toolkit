#ifndef BASE58CHECK_H
#define BASE58CHECK_H 1

int base58check_encode(char *, unsigned char *, size_t);
unsigned char *base58check_decode(char *, size_t, size_t *);
int base58check_valid_checksum(char *, size_t);

#endif
