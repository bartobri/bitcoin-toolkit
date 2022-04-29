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

static char *command = NULL;

int btk_help_init(int argc, char *argv[])
{
	if (argc <= 2)
	{
		return 1;
	}

	command = argv[2];

	return 1;
}

int btk_help_main(void)
{
	if (command == NULL)
	{
		btk_help_commands();
		return 1;
	}

	if (strcmp(command, "privkey") == 0)
	{
		btk_help_privkey();
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		btk_help_pubkey();
	}
	else if (strcmp(command, "node") == 0)
	{
		btk_help_node();
	}
	else if (strcmp(command, "vanity") == 0)
	{
		btk_help_vanity();
	}
	else if (strcmp(command, "version") == 0)
	{
		btk_help_version();
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("'%s' is not a valid command.", command);
		return -1;
	}
	
	return 1;
}

int btk_help_cleanup(void)
{
	return 1;
}

void btk_help_commands(void)
{
	printf("Usage: btk <command> [<args>]\n");
	printf("\n");
	printf("Here is a list of btk commands.\n");
	printf("\n");
	printf("   privkey      create, modify, and format private keys.\n");
	printf("   pubkey       calculate and format public keys from private keys.\n");
	printf("   vanity       generate a vanity address.\n");
	printf("   node         interface with a bitcoin node.\n");
	printf("   version      print btk version info.\n");
	printf("\n");
	printf("See 'btk help <command>' to read more about a specific command.\n");
}

void btk_help_privkey(void)
{
	printf("COMMAND\n");
	printf("\n");
	printf("   privkey - create, modify, and format private keys.\n");
	printf("\n");
	printf("SYNOPSIS\n");
	printf("\n");
	printf("   btk privkey [-n] [OUTPUT_OPTIONS]\n");
	printf("   btk privkey [INPUT_OPTIONS] [OUTPUT_OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   If -n is used, the privkey command creates a new random private key using\n");
	printf("   your local CSPRNG as an input source.\n");
	printf("\n");
	printf("   If an input option other than -n is specified, the privkey command will\n");
	printf("   read data from standard input and interpret its format according to the\n");
	printf("   input option. If no input option is specified, the privkey command will\n");
	printf("   read data from standard input and attempt to automatically determine the\n");
	printf("   format if it can be done with a reasonable degree of certainty.\n");
	printf("\n");
	printf("   If no output option is specified, the private key will be formatted in the\n");
	printf("   standard Wallet Import Format (WIF) by default. If an output option is\n");
	printf("   specified, the private key will be formatted according to the output\n");
	printf("   option. Private keys will be compressed by default, unless the input data\n");
	printf("   contains a compression flag, in which case it will remain consistent with\n");
	printf("   the input data. If an output option is specified for compression, the\n");
	printf("   private key will be compressed in accordance with that option, which will\n");
	printf("   override any compression flag provided in the input data.\n");
	printf("\n");
	printf("   See INPUT OPTIONS and OUTPUT OPTIONS for more info.\n");
	printf("\n");
	printf("INPUT OPTIONS\n");
	printf("\n");
	printf("   -n\n");
	printf("      Create a (n)ew random private key from your local CSPRNG.\n");
	printf("\n");
	printf("   -w\n");
	printf("      Treat input as a (w)allet import formatted (wif) string.\n");
	printf("\n");
	printf("   -h\n");
	printf("      Treat input as a (h)exadecimal formatted string. The string must be at\n");
	printf("      least 64 characters long. If a 65th and 66th character exist, they are\n");
	printf("      interpreted as a compression flag. Any extra characters beyond that point\n");
	printf("      are ignored.\n");
	printf("\n");
	printf("   -r\n");
	printf("      Treat input as (r)aw binary data. Input must be at least 32 bytes long.\n");
	printf("      If a 33rd byte exists, it is interpreted as a compression flag. Any\n");
	printf("      extra data beyond that point is ignored.\n");
	printf("\n");
	printf("   -s\n");
	printf("      Treat input as an arbitrary ASCII (s)tring. ASCII strings are processed\n");
	printf("      through a SHA256 hash algorithm to generate a 32 byte private key.\n");
	printf("\n");
	printf("   -d\n");
	printf("      Treat input as an ASCII string containing only (d)ecimal characters. The\n");
	printf("      decimal value is converted to its 32-byte binary equivalent to generate\n");
	printf("      a private key.\n");
	printf("\n");
	printf("   -b\n");
	printf("      Treat input as arbitrary (b)inary data. The data is processed through\n");
	printf("      a SHA256 hash algorithm to generate a 32 byte private key.\n");
	printf("\n");
	printf("   -x\n");
	printf("      Treat input as \"string binary data\". This option reads each\n");
	printf("      byte in the string, from right to left, and sets the corresponding\n");
	printf("      byte in the private key to the same value, from right to left. This\n");
	printf("      means that a string used with this option can not be longer than\n");
	printf("      32 bytes. When a string is less than 32 bytes, higher order bytes\n");
	printf("      of the private key are padded with null bytes.\n");
	printf("\n");
	printf("OUTPUT OPTIONS\n");
	printf("\n");
	printf("   -W\n");
	printf("      Print private key as a (W)allet import formatted string. (default)\n");
	printf("\n");
	printf("   -H\n");
	printf("      Print private key as a (H)exadecimal string. The string will be 64\n");
	printf("      characters in length representing the 32 byte private key. The string\n");
	printf("      will NOT contain a compression flag unless a compression option is\n");
	printf("      explicitely used, in which case the compression flag will be represented\n");
	printf("      in the 65th and 66th characters of the string.\n");
	printf("\n");
	printf("   -R\n");
	printf("      Print private key in (R)aw binary format. Output will be 32 bytes of\n");
	printf("      binary data. Output will NOT contain a compression flag unless a\n");
	printf("      compression option is explicitely used, in which case the compression\n");
	printf("      flag will be represented in 33rd byte.\n");
	printf("\n");
	printf("   -D\n");
	printf("      Print private key in (D)ecimal format. This is the literal decimal\n");
	printf("      equivalent  of the 32 byte private key, which in most cases will be a\n");
	printf("      very large number.\n");
	printf("\n");
	printf("   -C\n");
	printf("      (C)ompress the private key.\n");
	printf("\n");
	printf("   -U\n");
	printf("      (U)nompress the private key.\n");
	printf("\n");
	printf("   -T\n");
	printf("      If the -W option is specified (or assumed by default), this option sets\n");
	printf("      the (T)ESTNET prefix so the private key can be used on the TESTNET\n");
	printf("      network.\n");
	printf("\n");
	printf("   -M\n");
	printf("      If the -W option is specified (or assumed by default), this option sets\n");
	printf("      the (M)AINNET prefix so the private key can be used on the MAINNET\n");
	printf("      network. Note that if no network flag is specified or included in the\n");
	printf("      input data, the MAINNET prefix is used by default. This option is only\n");
	printf("      useful for converting TESTNET keys to MAINNET.\n");
	printf("\n");
	printf("   -N\n");
	printf("      Do NOT include a (N)ewline character in the output. This may be desirable\n");
	printf("      if the output is being parsed by a wrapper program.\n");
	printf("\n");
	printf("See https://github.com/bartobri/bitcoin-toolkit for examples.\n");
	printf("See 'btk help' to read about other commands.\n");
	printf("\n");
}

void btk_help_pubkey(void)
{
	printf("COMMAND\n");
	printf("\n");
	printf("   pubkey - calculate and format public keys from private keys.\n");
	printf("\n");
	printf("SYNOPSIS\n");
	printf("\n");
	printf("   btk pubkey [INPUT_OPTIONS] [OUTPUT_OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION");
	printf("\n");
	printf("   The pubkey command generates the corresponding public key from a given\n");
	printf("   private key. Input data should be a private key, or data from which a\n");
	printf("   private key can be derived, in the format specified by the input option.\n");
	printf("\n");
	printf("   The pubkey command will read data from standard input and interpret the\n");
	printf("   format according to the input option specified. If no input option is\n");
	printf("   specified, the pubkey command will read data from standard input and\n");
	printf("   attempt to automatically determine the format if it can be done with a\n");
	printf("   reasonable degree of certainty.\n");
	printf("\n");
	printf("   The pubkey command will calculate a public key from the provided input data\n");
	printf("   and print it in the format specified by the output option. If no output\n");
	printf("   option is specified, the public key will be formatted as a standard\n");
	printf("   bitcoin address. The public key will be compressed by default, unless\n");
	printf("   the input data contains a compression flag, in which case it will remain\n");
	printf("   consistent with the input data. If an output option is specified for\n");
	printf("   compression, the private key will be compressed in accordance with that\n");
	printf("   option, which will override any compression flag provided in the input data.\n");
	printf("\n");
	printf("   See INPUT OPTIONS and OUTPUT OPTIONS for more info.\n");
	printf("\n");
	printf("INPUT OPTIONS\n");
	printf("\n");
	printf("   -w\n");
	printf("      Treat input as a (w)allet import formatted (wif) string.\n");
	printf("\n");
	printf("   -h\n");
	printf("      Treat input as a (h)exadecimal formatted string. The string must be at\n");
	printf("      least 64 characters long. If a 65th and 66th character exist, they are\n");
	printf("      interpreted as a compression flag. Any extra characters beyond that point\n");
	printf("      are ignored.\n");
	printf("\n");
	printf("   -r\n");
	printf("      Treat input as (r)aw binary data. Input must be at least 32 bytes long.\n");
	printf("      If a 33rd byte exists, it is interpreted as a compression flag. Any\n");
	printf("      extra data beyond that point is ignored.\n");
	printf("\n");
	printf("   -s\n");
	printf("      Treat input as an arbitrary ASCII (s)tring. ASCII strings are processed\n");
	printf("      through a SHA256 hash algorithm to generate a 32 byte private key.\n");
	printf("\n");
	printf("   -d\n");
	printf("      Treat input as an ASCII string containing only (d)ecimal characters. The\n");
	printf("      decimal value is converted to its 32-byte binary equivalent to generate\n");
	printf("      a private key.\n");
	printf("\n");
	printf("   -b\n");
	printf("      Treat input as arbitrary (b)inary data. The data is processed through\n");
	printf("      a SHA256 hash algorithm to generate a 32 byte private key.\n");
	printf("\n");
	printf("OUTPUT OPTIONS\n");
	printf("\n");
	printf("   -A\n");
	printf("      Print public key as a traditional bitcoin (A)ddress. (default)\n");
	printf("\n");
	printf("   -B\n");
	printf("      Print public key as a (B)ech32 address.\n");
	printf("\n");
	printf("   -H\n");
	printf("      Print public key as a (H)exadecimal string. Note that a compressed\n");
	printf("      public key in hexadecimal format is 66 characters long. An uncompressed\n");
	printf("      public key is 130 characters long.\n");
	printf("\n");
	printf("   -R\n");
	printf("      Print public key as (R)aw binary data. This will result in 33 bytes for\n");
	printf("      a compressed public key and 65 bytes for an uncompressed public key.\n");
	printf("\n");
	printf("   -C\n");
	printf("      (C)ompress the public key. If -P is specified (see below), the\n");
	printf("      corresponding private key will also be compressed.\n");
	printf("\n");
	printf("   -U\n");
	printf("      (U)nompress the public key. If -P is specified (see below), the\n");
	printf("      corresponding private key will also be uncompressed.\n");
	printf("\n");
	printf("   -P\n");
	printf("      Include the (P)rivate key in the output. This option is most useful for\n");
	printf("      wrapper programs that want to generate and parse a private and public\n");
	printf("      key pair in a single command. The private key will be printed first,\n");
	printf("      followed by a single space, followed by the public key.\n");
	printf("\n");
	printf("   -N\n");
	printf("      Do NOT print a (N)ewline character. This may be a desirable option if\n");
	printf("      the output is being parsed by a wrapper program.\n");
	printf("\n");
	printf("   -T\n");
	printf("      If the -A or -B option is specified (or assumed by default), this option\n");
	printf("      sets the (T)ESTNET prefix so the public key is usable on the TESTNET\n");
	printf("      network. If the -P option is also specified, the private key will be\n");
	printf("      formatted for TESTNET network.\n");
	printf("\n");
	printf("   -M\n");
	printf("      If the -A or -B option is specified (or assumed by default), this option\n");
	printf("      sets the (M)AINNET prefix so the public key can be used on the MAINNET\n");
	printf("      network. Note that if no network flag is specified or included in the\n");
	printf("      input data, the MAINNET prefix is used by default. This option is only\n");
	printf("      useful for converting TESTNET keys to MAINNET. If the -P option is also\n");
	printf("      specified, the private key will be formatted for TESTNET network.\n");
	printf("\n");
	printf("See https://github.com/bartobri/bitcoin-toolkit for examples.\n");
	printf("See 'btk help' to read about other commands.\n");
	printf("\n");
}

void btk_help_vanity(void)
{
	printf("COMMAND\n");
	printf("\n");
	printf("   vanity - generate a vanity address.\n");
	printf("\n");
	printf("SYNOPSIS\n");
	printf("\n");
	printf("   btk vanity [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   The vanity command will read an input string from standard input and\n");
	printf("   continuously generate bitcoin addresses until it finds one whose initial\n");
	printf("   non-static characters match the input string provided. Note that the vanity\n");
	printf("   command attempts to match your input string after any static character\n");
	printf("   components of the address. Only include the vanity characters in your input\n");
	printf("   string. Do not include preceding static characters.\n");
	printf("\n");
	printf("   If no options are specified, the vanity command will search for a\n");
	printf("   traditional compressed bitcoin address. Options exist for uncompressed,\n");
	printf("   bech32, and testnet addresses. If the -i option is specified, the vanity\n");
	printf("   command will perform a case insensitive match\n");
	printf("\n");
	printf("   See OPTIONS for more info.\n");
	printf("\n");
	printf("OPTIONS\n");
	printf("\n");
	printf("   -A\n");
	printf("      Match a traditional bitcoin (A)ddress. (default)\n");
	printf("\n");
	printf("   -B\n");
	printf("      Match a (B)ech32 address.\n");
	printf("\n");
	printf("   -C\n");
	printf("      Match a (C)ompressed address. (default)\n");
	printf("\n");
	printf("   -U\n");
	printf("      Match an (U)ncompressed address. Note that this option is not allowed for\n");
	printf("      bech32 addresses.\n");
	printf("\n");
	printf("   -T\n");
	printf("      Generate a TESTNET address.\n");
	printf("\n");
	printf("   -i\n");
	printf("      Perform a case (i)nsensitive match. Note that this option is not useful\n");
	printf("      for bech32 addresses as all characters are lowercase.\n");
	printf("\n");
	printf("See https://github.com/bartobri/bitcoin-toolkit for examples.\n");
	printf("See 'btk help' to read about other commands.\n");
	printf("\n");
}

void btk_help_node(void)
{
	printf("COMMAND\n");
	printf("\n");
	printf("   node - interface with a bitcoin node.\n");
	printf("\n");
	printf("SYNOPSIS\n");
	printf("\n");
	printf("   btk node [-h <hostname>] [OPTIONS]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   The node command connects to and sends a bitcoin 'version' command to a\n");
	printf("   remote bitcoin node. It then dumps the response data to standard output in\n");
	printf("   JSON format. Bitcoin's version information contains the bitcoin protocol\n");
	printf("   version, available services, current block height, user agent string, and\n");
	printf("   more.\n");
	printf("\n");
	printf("   The node command will connect on the standard bitcoin port of 8333. If the\n");
	printf("   -T option is specified it will attempt to connect on port 18333 for TESTNET\n");
	printf("   nodes. If the -p option is specified, the port number that is specified as\n");
	printf("   its argument will override both of these defaults.\n");
	printf("\n");
	printf("   See OPTIONS for more info.\n");
	printf("\n");
	printf("OPTIONS\n");
	printf("\n");
	printf("   -h <hostname>\n");
	printf("      Specify the remote host to connect to. This is a required option.\n");
	printf("\n");
	printf("   -p <port number>\n");
	printf("      Specify the port number to connect to.\n");
	printf("\n");
	printf("   -T\n");
	printf("      This option is required if the remote host is a TESTNET node.\n");
	printf("\n");
	printf("See https://github.com/bartobri/bitcoin-toolkit for examples.\n");
	printf("See 'btk help' to read about other commands.\n");
	printf("\n");
}

void btk_help_version(void)
{
	printf("COMMAND\n");
	printf("\n");
	printf("   version - print btk version info.\n");
	printf("\n");
	printf("SYNOPSIS\n");
	printf("\n");
	printf("   btk version\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");
	printf("   Prints btk version information.\n");
	printf("\n");
	printf("OPTIONS\n");
	printf("\n");
	printf("   None\n");
	printf("\n");
	printf("See https://github.com/bartobri/bitcoin-toolkit for examples.\n");
	printf("See 'btk help' to read about other commands.\n");
	printf("\n");
}
