#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_LENGTH 65

typedef struct {
	unsigned char data[PUBKEY_LENGTH];
} PubKey;

PubKey pubkey_get(PrivKey);

#endif
