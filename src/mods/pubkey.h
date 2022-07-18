/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef PUBKEY_H
#define PUBKEY_H 1

#include "privkey.h"

#define PUBKEY_UNCOMPRESSED_LENGTH    64
#define PUBKEY_COMPRESSED_LENGTH      32

typedef struct PubKey *PubKey;

int pubkey_get(PubKey, PrivKey);
int pubkey_from_hex(PubKey, char *);
int pubkey_from_raw(PubKey, unsigned char *, size_t);
int pubkey_from_guess(PubKey, unsigned char *, size_t);
int pubkey_compress(PubKey);
int pubkey_uncompress(PubKey);
int pubkey_is_compressed(PubKey);
int pubkey_to_hex(char *, PubKey);
int pubkey_to_raw(unsigned char *, PubKey);
size_t pubkey_sizeof(void);

#endif
