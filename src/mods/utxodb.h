/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef UTXODB_H
#define UTXODB_H 1

#include <stdint.h>

#define UTXODB_KEY_TYPE                 0x43
#define UTXODB_KEY_MIN_LENGTH           34
#define UTXODB_KEY_MAX_LENGTH           38
#define UTXODB_TX_HASH_LENGTH           32

typedef struct UTXODBKey *UTXODBKey;
typedef struct UTXODBValue *UTXODBValue;

int utxodb_open(char *);
void utxodb_close(void);
int utxodb_obfuscate_key_get(void);
int utxodb_get(UTXODBKey, UTXODBValue, unsigned char *);

size_t utxodb_sizeof_key(void);
size_t utxodb_sizeof_value(void);
int utxodb_value_has_address(UTXODBValue);
int utxodb_value_has_compressed_pubkey(UTXODBValue);
int utxodb_value_has_uncompressed_pubkey(UTXODBValue);
int utxodb_value_get_address(char *, UTXODBValue);
void utxodb_value_free(UTXODBValue);
int utxodb_serialize_key(unsigned char *, size_t *, UTXODBKey);
int utxodb_set_value_from_raw(UTXODBValue, unsigned char *, size_t);
int utxodb_set_key_from_raw(UTXODBKey, unsigned char *, size_t);
int utxodb_set_key(UTXODBKey, unsigned char *, int);
int utxodb_set_key_type(UTXODBKey, char);
int utxodb_set_key_tx_hash(UTXODBKey, unsigned char *);
int utxodb_set_key_vout(UTXODBKey, int);
size_t utxodb_value_get_script_len(UTXODBValue);
uint64_t utxodb_value_get_height(UTXODBValue);
uint64_t utxodb_value_get_amount(UTXODBValue);
uint64_t utxodb_value_get_n_size(UTXODBValue);
int utxodb_value_get_script(unsigned char *, UTXODBValue);
uint64_t utxodb_key_get_vout(UTXODBKey);
int utxodb_key_get_tx_hash(unsigned char *, UTXODBKey);

#endif