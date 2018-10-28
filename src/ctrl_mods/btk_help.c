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
	printf("   Note that all btk input options are designed such that input should be\n");
	printf("   provided via pipe or redirection. None of these options take input as an\n");
	printf("   argument on the command line. In the absence of data via pipe or\n");
	printf("   redirection, btk will prompt the user for input if the input option\n");
	printf("   requires ASCII-only data.\n");
	printf("\n");
	printf("   -n         Create a (n)ew random private key from your local CSPRNG.\n");
	printf("   -w         Input is a (w)if formatted string.\n");
	printf("   -h         Input is a (h)exadecimal formatted string. Must be at least 64\n");
	printf("                 bytes long.\n");
	printf("   -r         Input is (r)aw binary data. Must be at least 32 bytes long.\n");
	printf("   -s         Input is an ASCII (s)tring. ASCII strings are processed through\n");
	printf("                 a SHA256 hashing algorithm to generate a 32 byte private key.\n");
	printf("   -d         Input is a (d)ecimal string. The decimal value is converted to\n");
	printf("                 the 32 byte binary equivalent to generate a private key.\n");
	printf("   -b         Input is arbitrary (b)inary data. The data is processed\n");
	printf("                 through a SHA256 hashing algorithm to generate a 32 byte\n");
	printf("                 private key.\n");
	printf("\n");
	printf("   If no input option is specified, btk will attempt to guess the input\n");
	printf("   option, based on input contents, in the following order: (1) If the input\n");
	printf("   is all numerical characters, btk assumes the -d option; (2) If the input is\n");
	printf("   exactly 64 hexadecimal characters, btk assumes the -h option; (3) If the\n");
	printf("   input is a properly encoded wif string with a valid checksum, btk assumes\n");
	printf("   the -w option; (4) If the input is all ASCII characters, btk assumes the -s\n");
	printf("   option; (5) If the input is exactly 32 bytes long, btk assumes the -r\n");
	printf("   option; (6) If all the above checks fail, btk will display an error message\n");
	printf("   requiring you to specify an input option.\n");
	printf("\n");
	printf("OUTPUT OPTIONS\n");
	printf("\n");
	printf("   -W         Print private key as a (W)if formmated string. (default)\n");
	printf("   -H         Print private key as a (H)exadecimal string.\n");
	printf("   -R         Print private key as in (R)aw binary form.\n");
	printf("   -D         Print private key as a *potentially very large* (D)ecimal.\n");
	printf("\n");
	printf("   If no output format option is specified, -W is used by default.\n");
	printf("\n");
	printf("   -C         (C)ompress the private key.\n");
	printf("   -U         (U)nompress the private key.\n");
	printf("\n");
	printf("   If no compression option is specified, the private key will be compressed\n");
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
	printf("DESCRIPTION");
	printf("\n");
	printf("   The pubkey command generates a corresponding public key any given private\n");
	printf("   key when the private key is provided as input, or when data is provided as\n");
	printf("   input from which a private key can be generated. The number of input\n");
	printf("   options for private keys allows for the creation of their corresponding\n");
	printf("   public keys in all kinds of fun and interesting ways. It also supports a\n");
	printf("   variety of output formats, compression, and mainnet/testnet options. See\n");
	printf("   each option description for more detail.\n");
	printf("\n");
	printf("INPUT OPTIONS\n");
	printf("\n");
	printf("   Note that all btk input options are designed such that input should be\n");
	printf("   provided via pipe or redirection. None of these options take input as an\n");
	printf("   argument on the command line. In the absence of data via pipe or\n");
	printf("   redirection, btk will prompt the user for input if the input option\n");
	printf("   requires ASCII-only data.\n");
	printf("\n");
	printf("   -n         Create a (n)ew random private key from your local CSPRNG.\n");
	printf("   -w         Input is a (w)if formatted string.\n");
	printf("   -h         Input is a (h)exadecimal formatted string. Must be at least 64\n");
	printf("                 bytes long.\n");
	printf("   -r         Input is (r)aw binary data. Must be at least 32 bytes long.\n");
	printf("   -s         Input is an ASCII (s)tring. ASCII strings are processed through\n");
	printf("                 a SHA256 hashing algorithm to generate a 32 byte private key.\n");
	printf("   -d         Input is a (d)ecimal string. The decimal value is converted to\n");
	printf("                 the 32 byte binary equivalent to generate a private key.\n");
	printf("   -b         Input is arbitrary (b)inary data. The data is processed\n");
	printf("                 through a SHA256 hashing algorithm to generate a 32 byte\n");
	printf("                 private key.\n");
	printf("\n");
	printf("   If no input option is specified, btk will attempt to guess the input\n");
	printf("   option, based on input contents, in the following order: (1) If the input\n");
	printf("   is all numerical characters, btk assumes the -d option; (2) If the input is\n");
	printf("   exactly 64 hexadecimal characters, btk assumes the -h option; (3) If the\n");
	printf("   input is a properly encoded wif string with a valid checksum, btk assumes\n");
	printf("   the -w option; (4) If the input is all ASCII characters, btk assumes the -s\n");
	printf("   option; (5) If the input is exactly 32 bytes long, btk assumes the -r\n");
	printf("   option; (6) If all the above checks fail, btk will display an error message\n");
	printf("   requiring you to specify an input option.\n");
	printf("\n");
	printf("OUTPUT OPTIONS\n");
	printf("   -A   Print public key as a traditional bitcoin (A)ddress. (default)\n");
	printf("   -B   Print public key as a (B)ech32 address.\n");
	printf("   -H   Print public key as an unencoded (H)exadecimal string.\n");
	printf("   -R   Print public key as unencoded (R)aw binary data.\n");
	printf("\n");
	printf("   If no output format option is specified, -A is used by default.\n");
	printf("\n");
	printf("   -C         (C)ompress the private/public keys.\n");
	printf("   -U         (U)nompress the private/public keys.\n");
	printf("\n");
	printf("   If no compression option is specified, the keys will be compressed\n");
	printf("   by default, unless a compression flag was provided in the input data (WIF\n");
	printf("   strings contain a compression flag), in which case it will match the input\n");
	printf("   setting.\n");
	printf("\n");
	printf("OTHER OPTIONS\n");
	printf("   -N   Do NOT print a newline character.\n");
	printf("   -P   Include the private key in the output.\n");
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
