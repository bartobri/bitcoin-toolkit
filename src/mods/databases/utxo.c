/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

// TODO - Add me to Makefile

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "mods/databases/utxo.h"
#include "mods/database.h"
#include "mods/hex.h"
#include "mods/serialize.h"
#include "mods/camount.h"
#include "mods/base58check.h"
#include "mods/error.h"

struct UTXOKey
{
    uint8_t        type;
    unsigned char  tx_hash[UTXO_TX_HASH_LENGTH];
    uint64_t       vout;
};

struct UTXOValue
{
    uint64_t       height;
    uint64_t       amount;
    uint64_t       n_size;
    size_t         script_len;
    unsigned char *script;
};

size_t utxo_sizeof_key(void)
{
    return sizeof(struct UTXOKey);
}

size_t utxo_sizeof_value(void)
{
    return sizeof(struct UTXOValue);
}

int utxo_value_has_address(UTXOValue value)
{
    assert(value);

    if (value->script != NULL)
    {
        // P2PKH (Public Key Hash)
        if (value->n_size == 0)
        {
            return 1;
        }

        // P2SH (Script Hash)
        if (value->n_size == 1)
        {
            return 1;
        }
    }

    return 0;
}

int utxo_value_has_compressed_pubkey(UTXOValue value)
{
    assert(value);

    if (value->script != NULL)
    {
        // Compressed Public Key (even)
        if (value->n_size == 2)
        {
            return 1;
        }

        // Compressed Public Key (odd)
        if (value->n_size == 3)
        {
            return 1;
        }
    }

    return 0;
}

int utxo_value_has_uncompressed_pubkey(UTXOValue value)
{
    assert(value);

    if (value->script != NULL)
    {
        // Uncompressed Public Key (even)
        if (value->n_size == 4)
        {
            return 1;
        }

        // Uncompressed Public Key (odd)
        if (value->n_size == 5)
        {
            return 1;
        }
    }

    return 0;
}

int utxo_value_get_address(char *address, UTXOValue value)
{
    int r;
    unsigned char *rmd;

    assert(address);
    assert(value);

    if (!utxo_value_has_address(value))
    {
        error_log("Value does not have an address.");
        return -1;
    }

    rmd = malloc(value->script_len + 1);
    if (rmd == NULL)
    {
        error_log("Memory allocation error.");
        return -1;
    }

    if (value->n_size == 1)
    {
        rmd[0] = 0x05;
    }
    else
    {
        rmd[0] = 0x00;
    }
    
    memcpy(rmd + 1, value->script, value->script_len);
    r = base58check_encode(address, rmd, value->script_len + 1);
    if (r < 0)
    {
        error_log("Could not generate address from value data.");
        return -1;
    }

    free(rmd);

    return 1;
}

void utxo_value_free(UTXOValue value)
{
    assert(value);

    if (value->script != NULL)
    {
        free(value->script);
        value->script = NULL;
    }
}

int utxo_serialize_key(unsigned char *output, size_t *output_len, UTXOKey key)
{
    int i;
    unsigned char *head;
    unsigned char tmp[UTXO_TX_HASH_LENGTH];

    assert(output);
    assert(output_len);
    assert(key);

    // Reverse byte order of tx_hash for serialization
    for (i = 0; i < UTXO_TX_HASH_LENGTH; i++)
    {
        tmp[i] = key->tx_hash[UTXO_TX_HASH_LENGTH - 1 - i];
    }

    head = output;

    output = serialize_uint8(output, key->type, SERIALIZE_ENDIAN_BIG);
    output = serialize_uchar(output, tmp, UTXO_TX_HASH_LENGTH);
    output = serialize_varint(output, key->vout);

    *output_len = 0;
    while (head++ != output)
    {
        (*output_len) += 1;
    }

    return 1;
}

int utxo_set_value_from_raw(UTXOValue value, unsigned char *raw_value, size_t value_len)
{
    size_t i;
    unsigned char *head;

    assert(value);
    assert(raw_value);
    assert(value_len);

    head = raw_value;

    raw_value = deserialize_varint(&(value->height), raw_value);
    raw_value = deserialize_varint(&(value->amount), raw_value);
    raw_value = deserialize_varint(&(value->n_size), raw_value);
    if (value->n_size == 0 || value->n_size == 1)
    {
        value->script_len = 20;
    }
    else
    {
        for (i = 0; head + i != raw_value; i++)
            ;
        value->script_len = value_len - i;
    }
    value->script = malloc(value->script_len);
    if (value->script == NULL)
    {
        error_log("Unable to allocate memory for output script value.");
        return -1;
    }
    raw_value = deserialize_uchar(value->script, raw_value, value->script_len);

    // pop off the coinbase flag from height.
    value->height = value->height >> 1;

    // Decompress amount
    camount_decompress(&(value->amount), value->amount);

    return 1;
}

int utxo_set_key_from_raw(UTXOKey key, unsigned char *raw_key, size_t key_len)
{
    int i;
    unsigned char tmp[UTXO_TX_HASH_LENGTH];

    assert(key);
    assert(raw_key);
    assert(key_len);

    if (raw_key[0] != UTXO_KEY_TYPE)
    {
        error_log("Key type not recognized: %.2x", raw_key[0]);
        return -1;
    }

    if (key_len < UTXO_KEY_MIN_LENGTH || key_len > UTXO_KEY_MAX_LENGTH)
    {
        error_log("Key length unexpected. Length is %zu. Expected between %d and %d.", key_len, UTXO_KEY_MIN_LENGTH, UTXO_KEY_MAX_LENGTH);
        return -1;
    }

    raw_key = deserialize_uint8(&(key->type), raw_key, SERIALIZE_ENDIAN_BIG);
    raw_key = deserialize_uchar(key->tx_hash, raw_key, UTXO_TX_HASH_LENGTH);
    raw_key = deserialize_varint(&(key->vout), raw_key);

    // Reverse byte order of tx_hash
    for (i = 0; i < UTXO_TX_HASH_LENGTH; i++)
    {
        tmp[i] = key->tx_hash[UTXO_TX_HASH_LENGTH - 1 - i];
    }
    for (i = 0; i < UTXO_TX_HASH_LENGTH; i++)
    {
        key->tx_hash[i] = tmp[i];
    }

    return 1;
}

int utxo_set_key(UTXOKey key, unsigned char *tx_hash, int vout)
{
    int r;

    assert(key);
    assert(tx_hash);

    r = utxo_set_key_type(key, UTXO_KEY_TYPE);
    if (r < 0)
    {
        error_log("Error while setting UTXO key type.");
        return -1;
    }

    r = utxo_set_key_tx_hash(key, tx_hash);
    if (r < 0)
    {
        error_log("Error while setting UTXO key tx_hash.");
        return -1;
    }

    r = utxo_set_key_vout(key, vout);
    if (r < 0)
    {
        error_log("Error while setting UTXO key vout.");
        return -1;
    }

    return 1;
}

int utxo_set_key_type(UTXOKey key, char value)
{
    assert(key);
    assert(value);

    if (value != UTXO_KEY_TYPE)
    {
        error_log("Unknown key type: %.2x.", value);
        return -1;
    }

    key->type = (uint8_t)value;

    return 1;
}

int utxo_set_key_tx_hash(UTXOKey key, unsigned char *value)
{
    assert(key);
    assert(value);

    memcpy(key->tx_hash, value, UTXO_TX_HASH_LENGTH);

    return 1;
}

int utxo_set_key_vout(UTXOKey key, int value)
{
    assert(key);

    key->vout = (uint32_t)value;

    return 1;
}

size_t utxo_value_get_script_len(UTXOValue value)
{
    assert(value);

    return value->script_len;
}

uint64_t utxo_value_get_height(UTXOValue value)
{
    assert(value);

    return value->height;
}

uint64_t utxo_value_get_amount(UTXOValue value)
{
    assert(value);

    return value->amount;
}

uint64_t utxo_value_get_n_size(UTXOValue value)
{
    assert(value);

    return value->n_size;
}

int utxo_value_get_script(unsigned char *script, UTXOValue value)
{
    assert(script);
    assert(value);

    if (value->script == NULL || value->script_len == 0)
    {
        error_log("Value object does not contain a script.");
        return -1;
    }

    memcpy(script, value->script, value->script_len);

    return 1;
}

uint64_t utxo_key_get_vout(UTXOKey key)
{
    assert(key);

    return key->vout;
}

int utxo_key_get_tx_hash(unsigned char *tx_hash, UTXOKey key)
{
    assert(tx_hash);
    assert(key);

    memcpy(tx_hash, key->tx_hash, UTXO_TX_HASH_LENGTH);

    return 1;
}
