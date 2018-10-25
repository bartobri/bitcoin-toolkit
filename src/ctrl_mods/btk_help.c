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
		btk_help_node();
	}
	else if (strcmp(argv[2], "vanity") == 0)
	{
		btk_help_vanity();
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
	printf("Usage: btk privkey [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   The privkey command can create and manipulate private keys in a variety of\n");
	printf("   fun and interesting ways. It supports compressed and uncompressed keys,\n");
	printf("   mainnet and testnet, and all sorts of input and output formats. See each\n");
	printf("   option description for more info.\n");
	printf("\n");
	printf("INPUT OPTIONS\n");
	printf("\n");
	printf("   Note that all btk input options are designed such that input should be provided\n");
	printf("   via pipe or redirection. none of these options take input as an argument on the\n");
	printf("   command line. In the absence of data via pipe or redirection, btk\n");
	printf("   may prompt the user for input if the input option allows for ASCII-only data\n");
	printf("   (as opposed to raw binary, for example).\n");
	printf("\n");
	printf("   -n         Create a (n)ew private key using 32 bytes of random data from your local CSPRNG.\n");
	printf("   -w         Input is a (w)if formatted string.\n");
	printf("   -h         Input is a 64 character (h)exidecimal string.\n");
	printf("   -r         Input is 32 bytes of (r)aw binary data.\n");
	printf("   -s         Input is an ASCII (s)tring that should be processed through a SHA256 hashing algorithm to generate a 32 byte private key.\n");
	printf("   -d         Input is a (d)ecimal string.\n");
	printf("   -b         Input is arbitrary (b)inary data that should be processed through a SHA256 hashing algorithm to generate a 32 byte private key.\n");
	printf("\n");
	printf("   If no input option is spectified, btk will attemmpt to guess the input option, based on input contents, in the following order:\n");
	printf("\n");
	printf("   1. If the input is all numerical characters, btk assumes the -d option.\n");
	printf("   2. If the input is exactly 64 hexidecimal characters, btk assumes the -h option.\n");
	printf("   3. If the input is a properly encoded wif string with a valid checksum, btk assumes the -w option.\n");
	printf("   4. If the input is all ASCII characters, btk assumes the -s option.\n");
	printf("   5. If the input is exactly 32 bytes long characters, btk assumes the -r option.\n");
	printf("   6. If all the above checks fail, btk will display an error message requiring you to specify an input option.\n");
	printf("\n");
	printf("OUTPUT OPTIONS\n");
	printf("\n");
	printf("   -W         Print private key as a (W)if formmated string. (default)\n");
	printf("   -H         Print private key as a (H)exidecimal string.\n");
	printf("   -R         Print private key as in (R)aw binary form.\n");
	printf("   -D         Print private key as a *potentially very large* (D)ecimal.\n");
	printf("\n");
	printf("   If no format option is spectified, -W is assumed by default.\n");
	printf("\n");
	printf("   -C         (C)ompress the private key. (default).\n");
	printf("   -U         (U)nompress the private key.\n");
	printf("\n");
	printf("   If no compression option is spectified, the private key will be compressed\n");
	printf("   by default, unless a compression flag was provided in the input data (WIF\n");
	printf("   strings contain a compression flag), in which case it will match the input\n");
	printf("   setting.\n");
	printf("\n");
	printf("OTHER OPTIONS\n");
	printf("   -N         Do NOT print a (N)ewline character at the end of the output.\n");
	printf("   -T         Format output for (T)ESTNET\n");
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

void btk_help_vanity(void)
{
	printf("Usage: btk vanity [OPTION]\n");
	printf("\n");
	printf("Description here.\n");
	printf("\n");
	printf("Match Options:\n");
	printf("   -i   Case insensitive match.\n");
	printf("\n");
	printf("Format Options:\n");
	printf("   -A   Traditional bitcoin address. (default)\n");
	printf("   -B   Bech32 address string.\n");
	printf("\n");
	printf("Compression Options:\n");
	printf("   -C   Compressed (default).\n");
	printf("   -U   Unompressed.\n");
	printf("\n");
	printf("Network Options:\n");
	printf("   -T   Format output for TESTNET\n");
	printf("\n");
}

void btk_help_node(void)
{
	printf("Usage: btk node [OPTION]\n");
	printf("\n");
	printf("Description here.\n");
	printf("\n");
	printf("Options:\n");
	printf("   -h   Hostname\n");
	printf("   -p   Port\n");
	printf("   -T   Use TESTNET\n");
	printf("\n");
}
