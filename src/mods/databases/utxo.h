/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef UTXO_H
#define UTXO_H 1

#define UTXO_DATABASE "/home/brian/tmp/leveldb_c/chainstate"
#define UTXO_KEY_TYPE 0x43

typedef struct UTXOKey *UTXOKey;
typedef struct UTXOValue *UTXOValue;

size_t utxo_sizeof_key(void);
size_t utxo_sizeof_value(void);
int utxo_open_database(char *);
int utxo_database_iter_seek_start(void);
int utxo_database_iter_get_next(UTXOKey, UTXOValue);
int utxo_get(UTXOValue, UTXOKey);
int utxo_value_has_address(UTXOValue);
int utxo_value_has_compressed_pubkey(UTXOValue);
int utxo_value_has_uncompressed_pubkey(UTXOValue);
int utxo_value_get_address(char *, UTXOValue);
int utxo_value_get_amount(uint64_t *, UTXOValue);
void utxo_value_free(UTXOValue);
void utxo_close_database(void);
int utxo_serialize_key(unsigned char *, size_t *, UTXOKey);
int utxo_set_value_from_raw(UTXOValue, unsigned char *, size_t);
int utxo_set_key_from_raw(UTXOKey, unsigned char *, size_t);
int utxo_set_key_from_hex(UTXOKey, char *, int);
int utxo_set_key(UTXOKey, unsigned char *, int);
int utxo_set_key_type(UTXOKey, char);
int utxo_set_key_tx_hash(UTXOKey, unsigned char *);
int utxo_set_key_vout(UTXOKey, int);
size_t utxo_value_get_script_len(UTXOValue);
uint64_t utxo_value_get_n_size(UTXOValue);
int utxo_value_get_script(unsigned char *, UTXOValue);

#endif