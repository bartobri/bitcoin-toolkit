#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_UNCOMPRESSED_LENGTH 64
#define PUBKEY_COMPRESSED_LENGTH   32

typedef struct {
	unsigned char data[PUBKEY_UNCOMPRESSED_LENGTH + 1];
} PubKey;

PubKey pubkey_get(PrivKey);
PubKey pubkey_compress(PubKey);
char  *pubkey_to_hex(PubKey);

#endif
