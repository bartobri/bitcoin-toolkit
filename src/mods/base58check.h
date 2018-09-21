#ifndef BASE58CHECK_H
#define BASE58CHECK_H 1

int base58check_encode(char *, unsigned char *, size_t);
int base58check_decode(unsigned char *, char *);
int base58check_valid_checksum(char *, size_t);

#endif
