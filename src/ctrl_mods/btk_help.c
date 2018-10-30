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
		error_log("See 'btk help' to read about available subcommands.");
		error_log("Invalid subcommand.");
		return -1;
	}
	
	return 1;
}

void btk_help_commands(void)
{
	printf("Usage: btk <subcommand> [OPTIONS]\n");
	printf("\n");
	printf("Here is a list of btk subcommands.\n");
	printf("\n");
	printf("   privkey      Create and modify private keys.\n");
	printf("   pubkey       Calculate the public key of a given private key.\n");
	printf("   vanity       Generate a vanity address.\n");
	printf("   node         Interface with bitcoin nodes.\n");
	printf("   version      Print btk version.\n");
	printf("\n");
	printf("See 'btk help <subcommand>' to read more about a specific subcommand.\n");
}

void btk_help_privkey(void)
{
	printf("Usage: btk privkey [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   The privkey subcommand can create and manipulate private keys in a variety of\n");
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
	printf("   -T         Set the (T)ESTNET flag.\n");
	printf("\n");
}

void btk_help_pubkey(void)
{
	printf("Usage: btk pubkey [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION");
	printf("\n");
	printf("   The pubkey subcommand generates a corresponding public key any given private\n");
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
	printf("   -N   Do NOT print a (N)ewline character.\n");
	printf("   -P   Include the (P)rivate key in the output.\n");
	printf("   -T   Set the (T)ESTNET flag.\n");
	printf("\n");
}

void btk_help_vanity(void)
{
	printf("Usage: btk vanity [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   The vanity subcommand finds vanity addresses by generating thousands of\n");
	printf("   addresses until it finds one whose initial characters match the input\n");
	printf("   string. Input should be provided via pipe or input redirection. Do not\n");
	printf("   provide input as a command line argument. In the absence of input via\n");
	printf("   redirection, btk will prompt the user to enter an input string. The vanity\n");
	printf("   subcommand supports both traditional and bech32 bitcoin addresses as well as\n");
	printf("   compression and testnet options.\n");
	printf("\n");
	printf("   Note that the vanity subcommand attempts to match your input string after any\n");
	printf("   static character components of the address. Do not include these static\n");
	printf("   characters in your input string.\n");
	printf("\n");
	printf("   About speed - As of the time of this writing, the vanity subcommand only\n");
	printf("   supports CPU operations. Until GPU support is added, it does not make much\n");
	printf("   sense to use this tool for matching strings longer than 4 or 5 characters.\n");
	printf("\n");
	printf("OPTIONS\n");
	printf("\n");
	printf("   Address Type\n");
	printf("\n");
	printf("   -A   Match a traditional bitcoin (A)ddress. (default)\n");
	printf("   -B   Match a (B)ech32 address.\n");
	printf("\n");
	printf("   Compression\n");
	printf("\n");
	printf("   -C   (C)ompressed (default).\n");
	printf("   -U   (U)nompressed.\n");
	printf("\n");
	printf("   Other\n");
	printf("\n");
	printf("   -T   Set the TESTNET flag.\n");
	printf("   -i   Case (i)nsensitive match.\n");
	printf("\n");
	printf("EXAMPLE\n");
	printf("\n");
	printf("   $ echo \"btc\" | btk vanity -i\n");
	printf("\n");
	printf("   This will attempt to find a traditional compressed bitcoin address that\n");
	printf("   starts with '1btc', '1BTC', or any variation of letter case.\n");
	printf("\n");
}

void btk_help_node(void)
{
	printf("Usage: btk node [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   At the moment of this writing the node subcommand performs a single function.\n");
	printf("   It connects to a remote bitcoin node and dumps the version message\n");
	printf("   information in a json formatted string. Aside from showing the node's\n");
	printf("   protocol version, a version message also contains information about\n");
	printf("   services supported by the node, the current block height of the node,\n");
	printf("   the user agent string, and more. The node subcommand supports both mainnet\n");
	printf("   and testnet.\n");
	printf("\n");
	printf("OPTIONS\n");
	printf("\n");
	printf("   -h   Hostname (required)\n");
	printf("   -p   Port (optional)\n");
	printf("   -T   Use TESTNET\n");
	printf("\n");
	printf("EXAMPLE\n");
	printf("\n");
	printf("   $ btk node -h seed.btc.petertodd.org\n");
	printf("\n");
}
