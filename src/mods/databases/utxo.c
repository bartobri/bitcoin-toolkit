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
#include <leveldb/c.h>
#include "mods/database.h"
#include "mods/databases/utxo.h"
#include "mods/hex.h"
#include "mods/serialize.h"
#include "mods/camount.h"
#include "mods/error.h"

#define OFUSCATE_KEY_KEY          "\016\000obfuscate_key"
#define OFUSCATE_KEY_KEY_LENGTH   15
#define TX_HASH_LENGTH            32
#define UTXO_KEY_MAX_LENGTH       38

unsigned char *obfuscate_key;
size_t obfuscate_key_len;

int utxo_get_obfuscate_key(void);

struct UTXOKey
{
    uint8_t        type;
    unsigned char  tx_hash[TX_HASH_LENGTH];
    uint32_t       vout;
};

struct UTXOValue
{
    uint64_t       height;
    uint64_t       amount;
    uint64_t       n_size;
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

int utxo_open_database(char *location)
{
    int r;

    r = database_open(location);

    if (r < 0)
    {
        error_log("Error while opening UTXO database.");
        return -1;
    }

    return 1;
}

int utxo_get(UTXOValue value, UTXOKey key)
{
    int r;
    size_t i;
    size_t serialized_key_len = 0;
    size_t output_len = 0;
    size_t script_len = 0;
    unsigned char serialized_key[UTXO_KEY_MAX_LENGTH];
    unsigned char *output = NULL;
    unsigned char *head;

    assert(value);
    assert(key);

    if (utxo_get_obfuscate_key() < 0) {
        error_log("Can not get obfuscate key.");
        return -1;
    }

    r = utxo_serialize_key(serialized_key, &serialized_key_len, key);
    if (r < 0 || serialized_key_len == 0) {
        error_log("Unable to serialize UTXO key.");
        return -1;
    }

    r = database_get(&output, &output_len, (char *)serialized_key, serialized_key_len);
    if (r < 0 || output_len == 0) {
        error_log("Unable to get value from database.");
        return -1;
    }

    // De-obfuscate the value
    for (i = 0; i < output_len; i++)
    {
        output[i] ^= obfuscate_key[i % obfuscate_key_len];
    }

    printf("Raw Value: ");
    for (i = 0; i < output_len; i++)
    {
        printf("%.2x", output[i]);
    }
    printf("\n");

    head = output;
    output = deserialize_varint(&(value->height), output);
    output = deserialize_varint(&(value->amount), output);
    output = deserialize_varint(&(value->n_size), output);
    // Getting number of bytes left for script.
    for (i = 0; head + i != output; i++)
        ;
    script_len = output_len - i;
    value->script = malloc(script_len);
    if (value->script == NULL)
    {
        error_log("Unable to allocate memory for output script value.");
        return -1;
    }
    output = deserialize_uchar(value->script, output, script_len);

    // pop off the coinbase flag from height.
    value->height = value->height >> 1;

    // Decompress amount
    camount_decompress(&(value->amount), value->amount);

    // If n_size is greater than 6, subtract 6 to get actual size.
    if (value->n_size >= 6)
    {
        value->n_size = value->n_size - 6;
    }






    /*
    printf("Obfuscation Key (%zu): ", obfuscate_key_len);
    for (i = 0; i < obfuscate_key_len; i++)
    {
        printf("%.2x", obfuscate_key[i]);
    }
    */
    printf("\n");
    printf("Height: %lu\n", value->height);
    printf("Amount: %lu\n", value->amount);
    printf("nSize: %lu\n", value->n_size);
    printf("Script (%lu): ", script_len);
    for (i = 0; i < script_len; i++)
    {
        printf("%.2x", value->script[i]);
    }
    printf("\n");


    return 1;
}

void utxo_close_database(void)
{
    database_close();
}

int utxo_get_obfuscate_key(void)
{
    int r;

    if (obfuscate_key == NULL)
    {
        r = database_get(&obfuscate_key, &obfuscate_key_len, OFUSCATE_KEY_KEY, OFUSCATE_KEY_KEY_LENGTH);
        if (r < 0) {
            error_log("Can not get obfuscate key.");
            return -1;
        }
        if (obfuscate_key == NULL || obfuscate_key_len == 0)
        {
            error_log("Can not get obfuscate key. No matching record found in database");
            return -1;
        }

        // Drop first char
        obfuscate_key++;
        obfuscate_key_len -= 1;
    }

    return 1;
}

int utxo_serialize_key(unsigned char *output, size_t *output_len, UTXOKey key)
{
    int i;
    unsigned char *head;
    unsigned char tmp[TX_HASH_LENGTH];

    assert(output);
    assert(output_len);
    assert(key);

    // Reverse byte order of tx_hash for serialization
    for (i = 0; i < TX_HASH_LENGTH; i++)
    {
        tmp[i] = key->tx_hash[TX_HASH_LENGTH - 1 - i];
    }

    head = output;

    output = serialize_uint8(output, key->type, SERIALIZE_ENDIAN_BIG);
    output = serialize_uchar(output, tmp, TX_HASH_LENGTH);
    output = serialize_varint(output, key->vout);

    *output_len = 0;
    while (head++ != output)
    {
        (*output_len) += 1;
    }

    return 1;
}

int utxo_set_key_from_hex(UTXOKey key, char *tx_hash_hex, int vout)
{
    int r;
    unsigned char tx_hash[TX_HASH_LENGTH];

    assert(key);
    assert(tx_hash_hex);

    if (strlen(tx_hash_hex) != 64)
    {
        error_log("UTXO tx hash in hex must be exactly 64 characters long.");
        return -1;
    }

    r = hex_str_to_raw(tx_hash, tx_hash_hex);
    if (r < 0)
    {
        error_log("Can not convert tx hex string to raw.");
        return -1;
    }

    r = utxo_set_key(key, tx_hash, vout);
    if (r < 0)
    {
        error_log("Can not set UTXO key values.");
        return -1;
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

    memcpy(key->tx_hash, value, TX_HASH_LENGTH);

    return 1;
}

int utxo_set_key_vout(UTXOKey key, int value)
{
    assert(key);

    key->vout = (uint32_t)value;

    return 1;
}