/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH         32
#define PRIVKEY_WIF_LENGTH_MIN 51
#define PRIVKEY_WIF_LENGTH_MAX 52
#define PRIVKEY_GUESS_DECIMAL  1
#define PRIVKEY_GUESS_HEX      2
#define PRIVKEY_GUESS_WIF      3
#define PRIVKEY_GUESS_STRING   4
#define PRIVKEY_GUESS_RAW      5
#define PRIVKEY_GUESS_BLOB     6

typedef struct PrivKey *PrivKey;

int privkey_new(PrivKey);
int privkey_compress(PrivKey);
int privkey_uncompress(PrivKey);
int privkey_to_hex(char *, PrivKey, int);
int privkey_to_raw(unsigned char *, PrivKey, int);
int privkey_to_wif(char *, PrivKey);
int privkey_to_dec(char *, PrivKey);
int privkey_from_wif(PrivKey, char *);
int privkey_from_hex(PrivKey, char *);
int privkey_from_dec(PrivKey, char *);
int privkey_from_sbd(PrivKey, char *);
int privkey_from_str(PrivKey, char *);
int privkey_from_raw(PrivKey, unsigned char *, size_t);
int privkey_from_blob(PrivKey, unsigned char *, size_t);
int privkey_from_guess(PrivKey, unsigned char *, size_t);
int privkey_is_compressed(PrivKey);
int privkey_is_zero(PrivKey);
size_t privkey_sizeof(void);
int privkey_rehash(PrivKey);

#endif
