#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_UNCOMPRESSED_LENGTH    64
#define PUBKEY_COMPRESSED_LENGTH      32

typedef struct PubKey *PubKey;

PubKey pubkey_get(PrivKey);
PubKey pubkey_compress(PubKey);
int    pubkey_is_compressed(PubKey);
char  *pubkey_to_hex(PubKey);
char  *pubkey_to_address(PubKey);
void   pubkey_free(PubKey);

#endif
