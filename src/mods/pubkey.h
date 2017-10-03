#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_LENGTH 65
#define PUBKEY_COMP_LENGTH 33

typedef struct {
	unsigned char data[PUBKEY_LENGTH];
} PubKey;

typedef struct {
	unsigned char data[PUBKEY_COMP_LENGTH];
} PubKeyComp;

PubKey     pubkey_get(PrivKey);
PubKeyComp pubkey_compress(PubKey);
PubKeyComp pubkey_get_compressed(PrivKeyComp);
char      *pubkey_to_hex(PubKey);
char      *pubkey_compressed_to_hex(PubKeyComp);

#endif
