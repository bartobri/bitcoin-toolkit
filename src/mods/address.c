/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "pubkey.h"
#include "privkey.h"
#include "bech32.h"
#include "network.h"
#include "base58check.h"
#include "crypto.h"
#include "error.h"

#define ADDRESS_VERSION_BIT_MAINNET      0x00
#define ADDRESS_VERSION_BIT_MAINNET_P2SH 0x05
#define ADDRESS_VERSION_BIT_TESTNET      0x6F

int address_get_p2pkh(char *address, PubKey key)
{
	int r;
	size_t len;
	unsigned char *data;
	unsigned char sha[32];
	unsigned char rmd[20];
	unsigned char rmd_bit[21];
	char base58[21 * 2];

	assert(address);
	assert(key);

	if (pubkey_is_compressed(key))
	{
		len = PUBKEY_COMPRESSED_LENGTH + 1;
	}
	else
	{
		len = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	}

	data = malloc(len);
	if (data == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = pubkey_to_raw(data, key);
	if (r < 0)
	{
		error_log("Could not get raw public key data.");
		return -1;
	}

	// RMD(SHA(data))
	r = crypto_get_sha256(sha, data, len);
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash from public key data.");
		return -1;
	}
	r = crypto_get_rmd160(rmd, sha, 32);
	if (r < 0)
	{
		error_log("Could not generate RMD160 hash from public key data.");
		return -1;
	}

	// Set address version bit
	if (network_is_main())
	{
		rmd_bit[0] = ADDRESS_VERSION_BIT_MAINNET;
	}
	else if (network_is_test())
	{
		rmd_bit[0] = ADDRESS_VERSION_BIT_TESTNET;
	}
	
	// Append rmd data
	memcpy(rmd_bit + 1, rmd, 20);
	
	r = base58check_encode(base58, rmd_bit, 21);
	if (r < 0)
	{
		error_log("Could not generate address from public key data.");
		return -1;
	}

	strcpy(address, base58);

	free(data);

	return 1;
}

int address_get_p2wpkh(char *address, PubKey key, int version)
{
	int r;
	int data_len;
	unsigned char *data;
	unsigned char sha[32];
	unsigned char rmd[20];

	assert(address);
	assert(key);
	assert(version == 0 || version == 1);  // 0 is bech32, 1 is bech32m (taproot)

	if (!pubkey_is_compressed(key))
	{
		error_log("Public key is uncompressed. Bech32 addresses require a compressed public key.");
		return -1;
	}

	data_len = PUBKEY_COMPRESSED_LENGTH + 1;
	data = malloc(data_len);
	if (data == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = pubkey_to_raw(data, key);
	if (r < 0)
	{
		error_log("Could not get raw public key data.");
		return -1;
	}

	// RMD(SHA(data))
	r = crypto_get_sha256(sha, data, data_len);
	if (r < 0)
	{
		error_log("Could not generate SHA256 hash from public key data.");
		return -1;
	}
	r = crypto_get_rmd160(rmd, sha, 32);
	if (r < 0)
	{
		error_log("Could not generate RMD160 hash from public key data.");
		return -1;
	}

	r = bech32_get_address(address, rmd, 20, version);
	if (r < 0)
	{
		error_log("Could not generate bech32 address from public key data.");
		return -1;
	}

	free(data);

	return 1;
}

int address_from_wif(char *address, char *wif)
{
	int r;
	PubKey pubkey = NULL;
	PrivKey privkey = NULL;

	assert(address);
	assert(wif);

	privkey = malloc(privkey_sizeof());
	if (privkey == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	pubkey = malloc(pubkey_sizeof());
	if (pubkey == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = privkey_from_wif(privkey, wif);
	if (r < 0)
	{
		error_log("Could not calculate private key from input.");
		return -1;
	}

	r = pubkey_get(pubkey, privkey);
	if (r < 0)
	{
		error_log("Could not calculate public key.");
		return -1;
	}

	r = address_get_p2pkh(address, pubkey);
	if (r < 0)
	{
		error_log("Could not calculate public key address.");
		return -1;
	}

	free(privkey);
	free(pubkey);

	return 1;
}

int address_from_str(char *address, char *str)
{
	int r;
	PubKey pubkey = NULL;
	PrivKey privkey = NULL;

	assert(address);
	assert(str);

	privkey = malloc(privkey_sizeof());
	if (privkey == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	pubkey = malloc(pubkey_sizeof());
	if (pubkey == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	r = privkey_from_str(privkey, str);
	if (r < 0)
	{
		error_log("Could not calculate private key from input.");
		return -1;
	}

	r = pubkey_get(pubkey, privkey);
	if (r < 0)
	{
		error_log("Could not calculate public key.");
		return -1;
	}

	r = address_get_p2pkh(address, pubkey);
	if (r < 0)
	{
		error_log("Could not calculate public key address.");
		return -1;
	}

	free(privkey);
	free(pubkey);

	return 1;
}

int address_from_rmd160(char *address, unsigned char *hash)
{
	int r;
	unsigned char rmd_bit[21];

	if (network_is_main())
	{
		rmd_bit[0] = ADDRESS_VERSION_BIT_MAINNET;
	}
	else if (network_is_test())
	{
		rmd_bit[0] = ADDRESS_VERSION_BIT_TESTNET;
	}

	memcpy(rmd_bit + 1, hash, 20);

	r = base58check_encode(address, rmd_bit, 21);
	ERROR_CHECK_NEG(r, "Could not generate address from public key data.");

	return 1;
}

int address_p2wpkh_from_raw(char *address, unsigned char *data, size_t len, int witver)
{
	int r;

	r = bech32_get_address(address, data, len, witver);
	ERROR_CHECK_NEG(r, "Could not generate P2WPKH (bech32) address.");

	return 1;
}

int address_from_sha256(char *address, unsigned char *hash)
{
	int r;
	unsigned char rmd[20];

	r = crypto_get_rmd160(rmd, hash, 32);
	ERROR_CHECK_NEG(r, "Could not generate RMD160 hash from public key data.");

	r = address_from_rmd160(address, rmd);
	ERROR_CHECK_NEG(r, "Could not get address from RMD160 hash.");

	return 1;
}

int address_from_p2sh_script(char *address, unsigned char *script)
{
	int r;
	unsigned char rmd_bit[21];

	rmd_bit[0] = ADDRESS_VERSION_BIT_MAINNET_P2SH;

	memcpy(rmd_bit + 1, script, 20);

	r = base58check_encode(address, rmd_bit, 21);
	ERROR_CHECK_NEG(r, "Could not generate address from script data.");

	return 1;
}