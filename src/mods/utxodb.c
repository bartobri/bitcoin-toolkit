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
#include <stdbool.h>
#include "mods/utxodb.h"
#include "mods/database.h"
#include "mods/hex.h"
#include "mods/serialize.h"
#include "mods/camount.h"
#include "mods/base58check.h"
#include "mods/error.h"

#define UTXODB_PATH_SIZE                1000
#define UTXODB_DEFAULT_PATH             ".bitcoin/chainstate"
#define UTXODB_OFUSCATE_KEY_KEY         "\016\000obfuscate_key"
#define UTXODB_OFUSCATE_KEY_KEY_LENGTH  15

struct UTXODBKey
{
    uint8_t        type;
    unsigned char  tx_hash[UTXODB_TX_HASH_LENGTH];
    uint64_t       vout;
};

struct UTXODBValue
{
    uint64_t       height;
    uint64_t       amount;
    uint64_t       n_size;
    size_t         script_len;
    unsigned char *script;
};

static DBRef dbref = -1;
static unsigned char *obfuscate_key = NULL;
static size_t obfuscate_key_len = 0;



int utxodb_open(char *p)
{
    int r;
    char path[UTXODB_PATH_SIZE];

    memset(path, 0, UTXODB_PATH_SIZE);

    if (p == NULL)
    {
        strcpy(path, getenv("HOME"));
        if (*path == 0)
        {
            error_log("Unable to determine home directory.");
            return -1;
        }
        strcat(path, "/");
        strcat(path, UTXODB_DEFAULT_PATH);
    }
    else
    {
        strcpy(path, p);
    }

    r = database_open(&dbref, path, false);
    if (r < 0)
    {
        error_log("Error while opening UTXO database.");
        return -1;
    }

    if (obfuscate_key == NULL)
    {
        r = utxodb_obfuscate_key_get();
        if (r < 0)
        {
            error_log("Could not get obfuscate key from database.");
            return -1;
        }
    }

    return 1;
}

void utxodb_close(void)
{
    if (dbref)
    {
        database_close(dbref);
    }
}

int utxodb_obfuscate_key_get(void)
{
    int r;

    assert(dbref >= 0);

    r = database_get(&obfuscate_key, &obfuscate_key_len, dbref, (unsigned char *)UTXODB_OFUSCATE_KEY_KEY, UTXODB_OFUSCATE_KEY_KEY_LENGTH);
    if (r < 0 || obfuscate_key == NULL || obfuscate_key_len == 0)
    {
        error_log("Database returned no data for obfuscate_key key.");
        return -1;
    }

    // Drop first char
    obfuscate_key++;
    obfuscate_key_len -= 1;

    return 1;
}

int utxodb_get(UTXODBKey key, UTXODBValue value, unsigned char *input)
{
    static bool init_seek = false;
    int r;
    size_t i;
    size_t serialized_key_len = 0;
    size_t serialized_value_len = 0;
    unsigned char *serialized_key = NULL;
    unsigned char *serialized_value = NULL;
    unsigned char tmp[UTXODB_TX_HASH_LENGTH];

    assert(key);
    assert(value);
    assert(input);
    assert(dbref >= 0);

    if (init_seek == false)
    {
        serialized_key = malloc(UTXODB_KEY_MAX_LENGTH);
        if (serialized_key == NULL)
        {
            error_log("Memory Allocation Error.");
            return -1;
        }

        r = utxodb_set_key(key, input, 0);
        if (r < 0)
        {
            error_log("Can not set UTXO database key values.");
            return -1;
        }

        r = utxodb_serialize_key(serialized_key, &serialized_key_len, key);
        if (r < 0 || serialized_key_len == 0) {
            error_log("Unable to serialize UTXO database key.");
            return -1;
        }

        r = database_iter_seek_key(dbref, serialized_key, serialized_key_len);
        if (r < 0) {
            error_log("Could not seek database iterator.");
            return -1;
        }
        else if (r == 0)
        {
            // End of database. Just return silently for now.
            return 0;
        }

        free(serialized_key);
        serialized_key = NULL;

        init_seek = true;
    }

    
    r = database_iter_get(&serialized_key, &serialized_key_len, &serialized_value, &serialized_value_len, dbref);
    if (r < 0)
    {
        error_log("Could not get data from database.");
        return -1;
    }

    // De-obfuscate the value
    for (i = 0; i < serialized_value_len; i++)
    {
        serialized_value[i] ^= obfuscate_key[i % obfuscate_key_len];
    }

    r = utxodb_set_key_from_raw(key, serialized_key, serialized_key_len);
    if (r < 0)
    {
        error_log("Could not deserialize raw key.");
        return -1;
    }

    r = utxodb_set_value_from_raw(value, serialized_value, serialized_value_len);
    if (r < 0)
    {
        error_log("Could not deserialize raw value.");
        return -1;
    }

    free(serialized_key);
    free(serialized_value);
    serialized_key = NULL;
    serialized_value = NULL;

    memset(tmp, 0, UTXODB_TX_HASH_LENGTH);

    r = utxodb_key_get_tx_hash(tmp, key);
    if (r < 0)
    {
        error_log("Unable to get TX hash from key.");
        return -1;
    }

    if (memcmp(input, tmp, UTXODB_TX_HASH_LENGTH) != 0)
    {
        memset(key, 0, utxodb_sizeof_key());
        memset(value, 0, utxodb_sizeof_value());
        return 0;
    }

    r = database_iter_next(dbref);
    if (r < 0)
    {
        error_log("Unable to set database iterator to next key.");
        return -1;
    }
    else if (r == 0)
    {
        return 0;
    }

    return 1;
}



size_t utxodb_sizeof_key(void)
{
    return sizeof(struct UTXODBKey);
}

size_t utxodb_sizeof_value(void)
{
    return sizeof(struct UTXODBValue);
}

int utxodb_value_has_address(UTXODBValue value)
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

int utxodb_value_has_compressed_pubkey(UTXODBValue value)
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

int utxodb_value_has_uncompressed_pubkey(UTXODBValue value)
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

int utxodb_value_get_address(char *address, UTXODBValue value)
{
    int r;
    unsigned char *rmd;

    assert(address);
    assert(value);

    if (!utxodb_value_has_address(value))
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

void utxodb_value_free(UTXODBValue value)
{
    assert(value);

    if (value->script != NULL)
    {
        free(value->script);
        value->script = NULL;
    }
}

int utxodb_serialize_key(unsigned char *output, size_t *output_len, UTXODBKey key)
{
    int i;
    unsigned char *head;
    unsigned char tmp[UTXODB_TX_HASH_LENGTH];

    assert(output);
    assert(output_len);
    assert(key);

    // Reverse byte order of tx_hash for serialization
    for (i = 0; i < UTXODB_TX_HASH_LENGTH; i++)
    {
        tmp[i] = key->tx_hash[UTXODB_TX_HASH_LENGTH - 1 - i];
    }

    head = output;

    output = serialize_uint8(output, key->type, SERIALIZE_ENDIAN_BIG);
    output = serialize_uchar(output, tmp, UTXODB_TX_HASH_LENGTH);
    output = serialize_varint(output, key->vout);

    *output_len = 0;
    while (head++ != output)
    {
        (*output_len) += 1;
    }

    return 1;
}

int utxodb_set_value_from_raw(UTXODBValue value, unsigned char *raw_value, size_t value_len)
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

int utxodb_set_key_from_raw(UTXODBKey key, unsigned char *raw_key, size_t key_len)
{
    int i;
    unsigned char tmp[UTXODB_TX_HASH_LENGTH];

    assert(key);
    assert(raw_key);
    assert(key_len);

    if (raw_key[0] != UTXODB_KEY_TYPE)
    {
        error_log("Key type not recognized: %.2x", raw_key[0]);
        return -1;
    }

    if (key_len < UTXODB_KEY_MIN_LENGTH || key_len > UTXODB_KEY_MAX_LENGTH)
    {
        error_log("Key length unexpected. Length is %zu. Expected between %d and %d.", key_len, UTXODB_KEY_MIN_LENGTH, UTXODB_KEY_MAX_LENGTH);
        return -1;
    }

    raw_key = deserialize_uint8(&(key->type), raw_key, SERIALIZE_ENDIAN_BIG);
    raw_key = deserialize_uchar(key->tx_hash, raw_key, UTXODB_TX_HASH_LENGTH);
    raw_key = deserialize_varint(&(key->vout), raw_key);

    // Reverse byte order of tx_hash
    for (i = 0; i < UTXODB_TX_HASH_LENGTH; i++)
    {
        tmp[i] = key->tx_hash[UTXODB_TX_HASH_LENGTH - 1 - i];
    }
    for (i = 0; i < UTXODB_TX_HASH_LENGTH; i++)
    {
        key->tx_hash[i] = tmp[i];
    }

    return 1;
}

int utxodb_set_key(UTXODBKey key, unsigned char *tx_hash, int vout)
{
    int r;

    assert(key);
    assert(tx_hash);

    r = utxodb_set_key_type(key, UTXODB_KEY_TYPE);
    if (r < 0)
    {
        error_log("Error while setting UTXO database key type.");
        return -1;
    }

    r = utxodb_set_key_tx_hash(key, tx_hash);
    if (r < 0)
    {
        error_log("Error while setting UTXO database ey tx_hash.");
        return -1;
    }

    r = utxodb_set_key_vout(key, vout);
    if (r < 0)
    {
        error_log("Error while setting UTXO database key vout.");
        return -1;
    }

    return 1;
}

int utxodb_set_key_type(UTXODBKey key, char value)
{
    assert(key);
    assert(value);

    if (value != UTXODB_KEY_TYPE)
    {
        error_log("Unknown key type: %.2x.", value);
        return -1;
    }

    key->type = (uint8_t)value;

    return 1;
}

int utxodb_set_key_tx_hash(UTXODBKey key, unsigned char *value)
{
    assert(key);
    assert(value);

    memcpy(key->tx_hash, value, UTXODB_TX_HASH_LENGTH);

    return 1;
}

int utxodb_set_key_vout(UTXODBKey key, int value)
{
    assert(key);

    key->vout = (uint32_t)value;

    return 1;
}

size_t utxodb_value_get_script_len(UTXODBValue value)
{
    assert(value);

    return value->script_len;
}

uint64_t utxodb_value_get_height(UTXODBValue value)
{
    assert(value);

    return value->height;
}

uint64_t utxodb_value_get_amount(UTXODBValue value)
{
    assert(value);

    return value->amount;
}

uint64_t utxodb_value_get_n_size(UTXODBValue value)
{
    assert(value);

    return value->n_size;
}

int utxodb_value_get_script(unsigned char *script, UTXODBValue value)
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

uint64_t utxodb_key_get_vout(UTXODBKey key)
{
    assert(key);

    return key->vout;
}

int utxodb_key_get_tx_hash(unsigned char *tx_hash, UTXODBKey key)
{
    assert(tx_hash);
    assert(key);

    memcpy(tx_hash, key->tx_hash, UTXODB_TX_HASH_LENGTH);

    return 1;
}
