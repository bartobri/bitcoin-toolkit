/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "pubkey.h"
#include "address.h"
#include "script.h"
#include "error.h"

#define MAX_OPS_PER_SCRIPT 201

typedef struct
{
	const char *word;
	// placeholder for function pointer
} Words;

Words words[256] = {
	[0x00].word = "OP_0",
	[0x01].word = "NA",
	[0x02].word = "NA",
	[0x03].word = "NA",
	[0x04].word = "NA",
	[0x05].word = "NA",
	[0x06].word = "NA",
	[0x07].word = "NA",
	[0x08].word = "NA",
	[0x09].word = "NA",
	[0x0a].word = "NA",
	[0x0b].word = "NA",
	[0x0c].word = "NA",
	[0x0d].word = "NA",
	[0x0e].word = "NA",
	[0x0f].word = "NA",
	[0x10].word = "NA",
	[0x11].word = "NA",
	[0x12].word = "NA",
	[0x13].word = "NA",
	[0x14].word = "NA",
	[0x15].word = "NA",
	[0x16].word = "NA",
	[0x17].word = "NA",
	[0x18].word = "NA",
	[0x19].word = "NA",
	[0x1a].word = "NA",
	[0x1b].word = "NA",
	[0x1c].word = "NA",
	[0x1d].word = "NA",
	[0x1e].word = "NA",
	[0x1f].word = "NA",
	[0x20].word = "NA",
	[0x21].word = "NA",
	[0x22].word = "NA",
	[0x23].word = "NA",
	[0x24].word = "NA",
	[0x25].word = "NA",
	[0x26].word = "NA",
	[0x27].word = "NA",
	[0x28].word = "NA",
	[0x29].word = "NA",
	[0x2a].word = "NA",
	[0x2b].word = "NA",
	[0x2c].word = "NA",
	[0x2d].word = "NA",
	[0x2e].word = "NA",
	[0x2f].word = "NA",
	[0x30].word = "NA",
	[0x31].word = "NA",
	[0x32].word = "NA",
	[0x33].word = "NA",
	[0x34].word = "NA",
	[0x35].word = "NA",
	[0x36].word = "NA",
	[0x37].word = "NA",
	[0x38].word = "NA",
	[0x39].word = "NA",
	[0x3a].word = "NA",
	[0x3b].word = "NA",
	[0x3c].word = "NA",
	[0x3d].word = "NA",
	[0x3e].word = "NA",
	[0x3f].word = "NA",
	[0x40].word = "NA",
	[0x41].word = "NA",
	[0x42].word = "NA",
	[0x43].word = "NA",
	[0x44].word = "NA",
	[0x45].word = "NA",
	[0x46].word = "NA",
	[0x47].word = "NA",
	[0x48].word = "NA",
	[0x49].word = "NA",
	[0x4a].word = "NA",
	[0x4b].word = "NA",
	[0x4c].word = "OP_PUSHDATA1",
	[0x4d].word = "OP_PUSHDATA2",
	[0x4e].word = "OP_PUSHDATA4",
	[0x4f].word = "OP_1NEGATE",
	[0x50].word = "OP_RESERVED",
	[0x51].word = "OP_TRUE",
	[0x52].word = "OP_2",
	[0x53].word = "OP_3",
	[0x54].word = "OP_4",
	[0x55].word = "OP_5",
	[0x56].word = "OP_6",
	[0x57].word = "OP_7",
	[0x58].word = "OP_8",
	[0x59].word = "OP_9",
	[0x5a].word = "OP_10",
	[0x5b].word = "OP_11",
	[0x5c].word = "OP_12",
	[0x5d].word = "OP_13",
	[0x5e].word = "OP_14",
	[0x5f].word = "OP_15",
	[0x60].word = "OP_16",
	[0x61].word = "OP_NOP",
	[0x62].word = "OP_VER",
	[0x63].word = "OP_IF",
	[0x64].word = "OP_NOTIF",
	[0x65].word = "OP_VERIF",
	[0x66].word = "OP_VERNOTIF",
	[0x67].word = "OP_ELSE",
	[0x68].word = "OP_ENDIF",
	[0x69].word = "OP_VERIFY",
	[0x6a].word = "OP_RETURN",
	[0x6b].word = "OP_TOALTSTACK",
	[0x6c].word = "OP_FROMALTSTACK",
	[0x6d].word = "OP_2DROP",
	[0x6e].word = "OP_2DUP",
	[0x6f].word = "OP_3DUP",
	[0x70].word = "OP_2OVER",
	[0x71].word = "OP_2ROT",
	[0x72].word = "OP_2SWAP",
	[0x73].word = "OP_IFDUP",
	[0x74].word = "OP_DEPTH",
	[0x75].word = "OP_DROP",
	[0x76].word = "OP_DUP",
	[0x77].word = "OP_NIP",
	[0x78].word = "OP_OVER",
	[0x79].word = "OP_PICK",
	[0x7a].word = "OP_ROLL",
	[0x7b].word = "OP_ROT",
	[0x7c].word = "OP_SWAP",
	[0x7d].word = "OP_TUCK",
	[0x7e].word = "OP_CAT",
	[0x7f].word = "OP_SUBSTR",
	[0x80].word = "OP_LEFT",
	[0x81].word = "OP_RIGHT",
	[0x82].word = "OP_SIZE",
	[0x83].word = "OP_INVERT",
	[0x84].word = "OP_AND",
	[0x85].word = "OP_OR",
	[0x86].word = "OP_XOR",
	[0x87].word = "OP_EQUAL",
	[0x88].word = "OP_EQUALVERIFY",
	[0x89].word = "OP_RESERVED1",
	[0x8a].word = "OP_RESERVED2",
	[0x8b].word = "OP_1ADD",
	[0x8c].word = "OP_1SUB",
	[0x8d].word = "OP_2MUL",
	[0x8e].word = "OP_2DIV",
	[0x8f].word = "OP_NEGATE",
	[0x90].word = "OP_ABS",
	[0x91].word = "OP_NOT",
	[0x92].word = "OP_0NOTEQUAL",
	[0x93].word = "OP_ADD",
	[0x94].word = "OP_SUB",
	[0x95].word = "OP_MUL",
	[0x96].word = "OP_DIV",
	[0x97].word = "OP_MOD",
	[0x98].word = "OP_LSHIFT",
	[0x99].word = "OP_RSHIFT",
	[0x9a].word = "OP_BOOLAND",
	[0x9b].word = "OP_BOOLOR",
	[0x9c].word = "OP_NUMEQUAL",
	[0x9d].word = "OP_NUMEQUALVERIFY",
	[0x9e].word = "OP_NUMNOTEQUAL",
	[0x9f].word = "OP_LESSTHAN",
	[0xa0].word = "OP_GREATERTHAN",
	[0xa1].word = "OP_LESSTHANOREQUAL",
	[0xa2].word = "OP_GREATERTHANOREQUAL",
	[0xa3].word = "OP_MIN",
	[0xa4].word = "OP_MAX",
	[0xa5].word = "OP_WITHIN",
	[0xa6].word = "OP_RIPEMD160",
	[0xa7].word = "OP_SHA1",
	[0xa8].word = "OP_SHA256",
	[0xa9].word = "OP_HASH160",
	[0xaa].word = "OP_HASH256",
	[0xab].word = "OP_CODESEPARATOR",
	[0xac].word = "OP_CHECKSIG",
	[0xad].word = "OP_CHECKSIGVERIFY",
	[0xae].word = "OP_CHECKMULTISIG",
	[0xaf].word = "OP_CHECKMULTISIGVERIFY",
	[0xb0].word = "OP_NOP1",
	[0xb1].word = "OP_CHECKLOCKTIMEVERIFY ",
	[0xb2].word = "OP_CHECKSEQUENCEVERIFY ",
	[0xb3].word = "OP_NOP4",
	[0xb4].word = "OP_NOP5",
	[0xb5].word = "OP_NOP6",
	[0xb6].word = "OP_NOP7",
	[0xb7].word = "OP_NOP8",
	[0xb8].word = "OP_NOP9",
	[0xb9].word = "OP_NOP10",
	
	[0xfa].word = "OP_SMALLINTEGER",
	[0xfb].word = "OP_PUBKEYS",

	[0xfd].word = "OP_PUBKEYHASH",
	[0xfc].word = "OP_PUBKEY",
	[0xff].word = "OP_INVALIDOPCODE",
};

const char *script_get_word(uint8_t w)
{
	// TODO - change this function to use a pointer as a parameter
	// for output and return an int for success or failure
	return words[w].word;
}

char *script_from_raw(unsigned char *raw, size_t l)
{
	// TODO - change this function to use a pointer as a parameter
	// for output and return an int for success or failure
	size_t c, i, j;
	unsigned char op;
	char *ops[MAX_OPS_PER_SCRIPT];
	char *r;
	
	for (c = 0, i = 0; i < l; ++i, ++c)
	{
		if (c >= MAX_OPS_PER_SCRIPT)
		{
			error_log("Script has more operations than the allowed limit of %i.", MAX_OPS_PER_SCRIPT);
			return NULL;
		}
		
		op = *raw;
		++raw;

		if (op >= 0x01 && op <= 0x4b)
		{
			if (i + op >= l)
			{
				error_log("Script length is too short for operation 0x%02x.", op);
				return NULL;
			}
			ops[c] = malloc(op * 2 + 1);
			if (ops[c] == NULL)
			{
				// TODO - handle memory allocation error
			}
			for (j = 0; j < op; ++j, ++raw)
			{
				sprintf(ops[c] + (j * 2), "%02x", *raw);
			}
			ops[c][j * 2] = '\0';
			i += op;
		//} else if (op == 0x4c) {
		//} else if (op == 0x4d) {
		//} else if (op == 0x4e) {
		}
		else
		{
			ops[c] = malloc(strlen(words[op].word) + 1);
			if (ops[c] == NULL)
			{
				// TODO - handle memory allocation error
			}
			strcpy(ops[c], words[op].word);
		}
	}

	for (j = 0, i = 0; i < c; ++i)
	{
		j += strlen(ops[i]) + 1;
	}
	r = malloc(j + 1);
	if (r == NULL)
	{
		// TODO - handle memory allocation error
	}
	memset(r, 0, j + 1);
	for (i = 0; i < c; ++i)
	{
		strcat(r, ops[i]);
		strcat(r, " ");
		free(ops[i]);
	}
	
	return r;
}

int script_get_output_address(char *address, unsigned char *script, uint64_t size, uint32_t tx_version)
{
	int r;
	unsigned char last_op;
	PubKey pubkey = NULL;
	int uc_pubkey_len = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	int c_pubkey_len = PUBKEY_COMPRESSED_LENGTH + 1;
	unsigned char tmp[BUFSIZ];

	// May need this in the future
	(void)tx_version;

	// Get last op to help identify some script types
	last_op = *(script + (size - 1));

	// Set address to null string so we can check it at the end of this function.
	*address = 0;

	// ===========================================
	// Verison 2 (caller checked for non-standard)
	// ===========================================

	// witness_v0_keyhash, 20 byte
	if (*script == 0x00 && *(script + 1) == 0x14 && (int)size == (1 + 1 + *(script + 1)))
	{
		r = address_p2wpkh_from_raw(address, script + 2, 0x14, 0);
		ERROR_CHECK_NEG(r, "Could not generate address from value data.");
	}
	// witness_v0_keyhash, 32 byte
	else if (*script == 0x00 && *(script + 1) == 0x20 && (int)size == (1 + 1 + *(script + 1)))
	{
		r = address_p2wpkh_from_raw(address, script + 2, 0x20, 0);
		ERROR_CHECK_NEG(r, "Could not generate address from value data.");
	}
	// witness_v1_taproot, 32 byte
	else if (*script == 0x51 && *(script + 1) == 0x20 && (int)size == (1 + 1 + *(script + 1)))
	{
		r = address_p2wpkh_from_raw(address, script + 2, 0x20, 1);
		ERROR_CHECK_NEG(r, "Could not generate address from value data.");
	}
	// OP_CHECKSIG
	else if (last_op == 0xac)
	{
		// Hash160
		if (*script == 0x76 && *(script + 1) == 0xa9 && *(script + 2) == 0x14)
		{
			r = address_from_rmd160(address, script + 3);
			ERROR_CHECK_NEG(r, "Could not generate address from value data.");
		}
		// Hash160 - all zeros - burner address?
		else if (*script == 0x76 && *(script + 1) == 0xa9 && *(script + 2) == 0x00)
		{
			memset(tmp, 0, BUFSIZ);

			r = address_from_rmd160(address, tmp);
			ERROR_CHECK_NEG(r, "Could not generate address from value data.");
		}
		// Uncompressed Public Key
		else if (*script == uc_pubkey_len && (int)size == uc_pubkey_len + 2)
		{
			script++;

			// Make sure the compression flag is correct
			if (*script == 0x04)
			{
				pubkey = malloc(pubkey_sizeof());

				r = pubkey_from_raw(pubkey, script, uc_pubkey_len);
				ERROR_CHECK_NEG(r, "Can not get pubkey object from output script.");

				r = address_get_p2pkh(address, pubkey);
				ERROR_CHECK_NEG(r, "Can not get address from pubkey.");

				free(pubkey);
			}
		}
		// Compressed Public Key
		else if (*script == c_pubkey_len && (int)size == c_pubkey_len + 2)
		{
			script++;

			// Make sure the compression flag is correct
			if (*script == 0x02 || *script == 0x03)
			{
				pubkey = malloc(pubkey_sizeof());

				r = pubkey_from_raw(pubkey, script, c_pubkey_len);
				ERROR_CHECK_NEG(r, "Can not get pubkey object from output script.");

				r = address_get_p2pkh(address, pubkey);
				ERROR_CHECK_NEG(r, "Can not get address from pubkey.");

				free(pubkey);
			}
		}
	}
	// OP_EQUAL
	else if (last_op == 0x87)
	{
		// P2SH - OP_HASH160
		if (*script == 0xa9 && *(script + 1) == 0x14 && size == 0x14 + 3)
		{
			r = address_from_p2sh_script(address, script + 2);
			ERROR_CHECK_NEG(r, "Could not generate address.");
		}
	}

	// If we don't find an address, return 0 and let caller decide what to do.
	if (!(*address))
	{
		return 0;
	}

	return 1;
}
