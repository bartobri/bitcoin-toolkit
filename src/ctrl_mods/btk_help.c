/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btk_help.h"
#include "mods/error.h"

int btk_help_main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		btk_help_commands();
		return 1;
	}

	if (strcmp(argv[2], "privkey") == 0)
	{
		btk_help_privkey();
	}
	else if (strcmp(argv[2], "pubkey") == 0)
	{
		btk_help_pubkey();
	}
	else if (strcmp(argv[2], "node") == 0)
	{
		printf("node help here\n");
	}
	else if (strcmp(argv[2], "vanity") == 0)
	{
		printf("vanity help here\n");
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Invalid command.");
		return -1;
	}
	
	return 1;
}

void btk_help_commands(void)
{
	printf("usage: btk <command> [OPTION]\n");
	printf("\n");
	printf("Here is a list of btk commands.\n");
	printf("\n");
	printf("   privkey      Create and/or manipulate private keys.\n");
	printf("   pubkey       Create and/or manipulate public keys and addresses.\n");
	printf("   vanity       Generate a vanity address.\n");
	printf("   node         Interface with bitcoin nodes.\n");
	printf("   version      Print btk version.\n");
	printf("\n");
	printf("See 'btk help <command>' to read more about a specific command.\n");
}

void btk_help_privkey(void)
{
	printf("Usage: btk privkey [OPTION]\n");
	printf("\n");
	printf("Description here.\n");
	printf("\n");
	printf("Input Options:\n");
	printf("   -n   Generate a new private key from your local CSPRNG source.\n");
	printf("   -w   Generate a private key from a WIF formatted string.\n");
	printf("   -h   Generate a private key from a 64 byte hexidecimal string.\n");
	printf("   -r   Generate a private key from a 32 byte raw binary string.\n");
	printf("   -s   Generate a private key from an ASCII string processed through a SHA256 hashing algorithm.\n");
	printf("   -d   Generate a private key from a decimal string.\n");
	printf("   -b   Generate a private key from arbitrary binary data processed through a SHA256 hashing algorithm.\n");
	printf("\n");
	printf("Output Format Options:\n");
	printf("   -W   Print private key as a WIF formmated string.\n");
	printf("   -H   Print private key as a 64-65 byte hexidecimal string.\n");
	printf("   -R   Print private key as 32 bytes of raw binary data.\n");
	printf("   -D   Print private key as a (very large) decimal.\n");
	printf("\n");
	printf("Output Compression Options:\n");
	printf("   -C   Compressed (default).\n");
	printf("   -U   Unompressed.\n");
	printf("\n");
	printf("Output Misc Options:\n");
	printf("   -N   Do NOT print a newline character.\n");
	printf("\n");
	printf("Network Options:\n");
	printf("   -T   Format output for TESTNET\n");
	printf("\n");
}

void btk_help_pubkey(void)
{
	printf("Usage: btk pubkey [OPTION]\n");
	printf("\n");
	printf("Description here.\n");
	printf("\n");
	printf("Input Options:\n");
	printf("   -w   Generate a private key from a WIF formatted string.\n");
	printf("   -h   Generate a private key from a 64 byte hexidecimal string.\n");
	printf("   -r   Generate a private key from a 32 byte raw binary string.\n");
	printf("   -s   Generate a private key from an ASCII string processed through a SHA256 hashing algorithm.\n");
	printf("   -d   Generate a private key from a decimal string.\n");
	printf("   -b   Generate a private key from arbitrary binary data processed through a SHA256 hashing algorithm.\n");
	printf("\n");
	printf("Output Format Options:\n");
	printf("   -A   Print public key as a traditional bitcoin address string. (default)\n");
	printf("   -B   Print public key as a bech32-encoded address string.\n");
	printf("   -H   Print public key as hexidecimal string.\n");
	printf("   -R   Print public key as raw binary data.\n");
	printf("\n");
	printf("Output Compression Options:\n");
	printf("   -C   Compressed (default).\n");
	printf("   -U   Unompressed.\n");
	printf("\n");
	printf("Output Misc Options:\n");
	printf("   -N   Do NOT print a newline character.\n");
	printf("   -P   Include the private key in the output.\n");
	printf("\n");
	printf("Network Options:\n");
	printf("   -T   Format output for TESTNET\n");
	printf("\n");
}
