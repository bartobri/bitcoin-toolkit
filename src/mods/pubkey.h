#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_UNCOMPRESSED_LENGTH    64
#define PUBKEY_COMPRESSED_LENGTH      32

typedef struct PubKey *PubKey;

int pubkey_get(PubKey, PrivKey);
int pubkey_compress(PubKey);
int    pubkey_is_compressed(PubKey);
char  *pubkey_to_hex(PubKey);
unsigned char *pubkey_to_raw(PubKey, size_t*);
char  *pubkey_to_address(PubKey);
char  *pubkey_to_bech32address(PubKey);
void   pubkey_free(PubKey);
size_t pubkey_sizeof(void);

#endif
