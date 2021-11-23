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
int pubkey_from_raw(PubKey key, unsigned char *input, size_t input_len);
int pubkey_compress(PubKey);
int pubkey_decompress(PubKey);
int pubkey_is_compressed(PubKey);
int pubkey_to_hex(char *, PubKey);
int pubkey_to_raw(unsigned char *, PubKey);
int pubkey_to_address(char *, PubKey);
int pubkey_to_bech32address(char *, PubKey);
int pubkey_address_from_wif(char *, char *);
int pubkey_address_from_str(char *, char *);
size_t pubkey_sizeof(void);

#endif
