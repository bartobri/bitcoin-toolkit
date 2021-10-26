/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef UTXO_H
#define UTXO_H 1

#include <stdint.h>

#define UTXO_PATH                     "/home/brian/tmp/leveldb_c/chainstate"
#define UTXO_KEY_TYPE                 0x43
#define UTXO_KEY_MIN_LENGTH           34
#define UTXO_KEY_MAX_LENGTH           38
#define UTXO_OFUSCATE_KEY_KEY         "\016\000obfuscate_key"
#define UTXO_OFUSCATE_KEY_KEY_LENGTH  15
#define UTXO_TX_HASH_LENGTH           32

typedef struct UTXOKey *UTXOKey;
typedef struct UTXOValue *UTXOValue;

size_t utxo_sizeof_key(void);
size_t utxo_sizeof_value(void);
int utxo_value_has_address(UTXOValue);
int utxo_value_has_compressed_pubkey(UTXOValue);
int utxo_value_has_uncompressed_pubkey(UTXOValue);
int utxo_value_get_address(char *, UTXOValue);
void utxo_value_free(UTXOValue);
int utxo_serialize_key(unsigned char *, size_t *, UTXOKey);
int utxo_set_value_from_raw(UTXOValue, unsigned char *, size_t);
int utxo_set_key_from_raw(UTXOKey, unsigned char *, size_t);
int utxo_set_key(UTXOKey, unsigned char *, int);
int utxo_set_key_type(UTXOKey, char);
int utxo_set_key_tx_hash(UTXOKey, unsigned char *);
int utxo_set_key_vout(UTXOKey, int);
size_t utxo_value_get_script_len(UTXOValue);
uint64_t utxo_value_get_height(UTXOValue);
uint64_t utxo_value_get_amount(UTXOValue);
uint64_t utxo_value_get_n_size(UTXOValue);
int utxo_value_get_script(unsigned char *, UTXOValue);
uint64_t utxo_key_get_vout(UTXOKey);
int utxo_key_get_tx_hash(unsigned char *, UTXOKey);

#endif