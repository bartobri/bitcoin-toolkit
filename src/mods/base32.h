#ifndef BASE32_H
#define BASE32_H 1

int base32_encode(char *, unsigned char *, size_t);
int base32_encode_raw(unsigned char *, unsigned char *, size_t);
int base32_get_char(int);
int base32_get_raw(char);

#endif