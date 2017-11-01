#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

typedef struct PubKey *PubKey;

PubKey pubkey_get(PrivKey);
PubKey pubkey_compress(PubKey);
int    pubkey_is_compressed(PubKey);
char  *pubkey_to_hex(PubKey);
char  *pubkey_to_address(PubKey);

#endif
