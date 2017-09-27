#ifndef ADDRESS_H
#define ADDRESS_H 1

#define PRIVATE_KEY_BYTES 32
#define PUBLIC_KEY_BYTES  65

void address_get(unsigned char *, unsigned char *);
void address_print(unsigned char *);

#endif
