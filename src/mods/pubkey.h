#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_UNCOMPRESSED_LENGTH    64
#define PUBKEY_COMPRESSED_LENGTH      32

typedef struct PubKey *PubKey;

int pubkey_get(PubKey, PrivKey);
int pubkey_compress(PubKey);
int pubkey_is_compressed(PubKey);
int pubkey_to_hex(char *, PubKey);
int pubkey_to_raw(unsigned char *, PubKey);
int pubkey_to_address(char *, PubKey);
char  *pubkey_to_bech32address(PubKey);
void   pubkey_free(PubKey);
size_t pubkey_sizeof(void);

#endif
