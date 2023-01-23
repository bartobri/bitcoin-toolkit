/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mods/input.h"
#include "mods/json.h"
#include "mods/opts.h"
#include "mods/error.h"
#include "ctrl_mods/btk_help.h"
#include "ctrl_mods/btk_privkey.h"
#include "ctrl_mods/btk_pubkey.h"
#include "ctrl_mods/btk_address.h"
#include "ctrl_mods/btk_node.h"
#include "ctrl_mods/btk_utxodb.h"
#include "ctrl_mods/btk_addressdb.h"
#include "ctrl_mods/btk_version.h"

#define BTK_CHECK_NEG(x, y)         if (x < 0) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_NULL(x, y)        if (x == NULL) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_FALSE(x, y)       if (!x) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_TRUE(x, y)        if (x) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }

int main(int argc, char *argv[])
{
	int i, r = 0;
	int json_len = 0;
	unsigned char *input = NULL; 
	size_t input_len = 0;
	char *command = NULL;
	char command_str[BUFSIZ];
	char json_str[BUFSIZ];
	opts_p opts = NULL;
	char *opts_string = NULL;
	int (*main_fp)(opts_p, unsigned char *, size_t) = NULL;
	int (*input_fp)(opts_p) = NULL;

	json_init();

	// Assembling the original command string for logging purposes
	memset(command_str, 0, BUFSIZ);
	for (i = 0; i < argc; ++i)
	{
		if (i > 0)
		{
			strncat(command_str, " ", BUFSIZ - 1 - strlen(command_str));
		}
		strncat(command_str, argv[i], BUFSIZ - 1 - strlen(command_str));
	}

	if (argc <= 1)
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("Missing command parameter.");
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}
	else
	{
		command = argv[1];
	}

	if (strcmp(command, "privkey") == 0)
	{
		opts_string = OPTS_STRING_PRIVKEY;
		main_fp = &btk_privkey_main;
		input_fp = &btk_privkey_requires_input;
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		opts_string = OPTS_STRING_PUBKEY;
		main_fp = &btk_pubkey_main;
		input_fp = &btk_pubkey_requires_input;
	}
	else if (strcmp(command, "address") == 0)
	{
		opts_string = OPTS_STRING_ADDRESS;
		main_fp = &btk_address_main;
		input_fp = &btk_address_requires_input;
	}
	else if (strcmp(command, "node") == 0)
	{
		opts_string = OPTS_STRING_NODE;
		main_fp = &btk_node_main;
		input_fp = &btk_node_requires_input;
	}
	else if (strcmp(command, "utxodb") == 0)
	{
		opts_string = OPTS_STRING_UTXODB;
		main_fp = &btk_utxodb_main;
		input_fp = &btk_utxodb_requires_input;
	}
	else if (strcmp(command, "addressdb") == 0)
	{
		opts_string = OPTS_STRING_ADDRESSDB;
		main_fp = &btk_addressdb_main;
		input_fp = &btk_addressdb_requires_input;
	}
	else if (strcmp(command, "version") == 0)
	{
		main_fp = &btk_version_main;
		input_fp = &btk_version_requires_input;
	}
	else if (strcmp(command, "help") == 0)
	{
		opts_string = OPTS_STRING_HELP;
		main_fp = &btk_help_main;
		input_fp = &btk_help_requires_input;
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("'%s' is not a valid command.", command);
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}

	opts = malloc(sizeof(*opts));
	if (opts == NULL)
	BTK_CHECK_NULL(opts, "Memory allocation error.");

	r = opts_init(opts);
	BTK_CHECK_NEG(r, NULL);

	if (opts_string)
	{
		r = opts_get(opts, argc, argv, opts_string);
		BTK_CHECK_NEG(r, NULL);
	}

	if (input_fp(opts))
	{
		r = input_get(&input, &input_len);
		BTK_CHECK_NEG(r, NULL);
	}

	if (input)
	{
		BTK_CHECK_TRUE(opts->input_format_binary && opts->input_format_list, "Can not use both binary and list input format opts.");

		if (opts->input_format_binary)
		{
			r = main_fp(opts, input, input_len);
			BTK_CHECK_NEG(r, NULL);
		}
		else if (opts->input_format_list)
		{
			char *tok;
			tok = strtok((char *)input, "\n");
			while (tok != NULL)
			{
				r = main_fp(opts, (unsigned char *)tok, strlen(tok));
				BTK_CHECK_NEG(r, NULL);

				tok = strtok(NULL, "\n");
			}
		}
		else
		{
			r = json_is_valid((char *)input, input_len);
			BTK_CHECK_FALSE(r, "Input contains invalid JSON formatting.");

			r = json_set_input((char *)input);
			BTK_CHECK_NEG(r, "Could not load JSON input.");

			r = json_get_input_len(&json_len);
			BTK_CHECK_NEG(r, "Could not get input list length.");

			for (i = 0; i < json_len; i++)
			{
				memset(json_str, 0, BUFSIZ);

				r = json_get_input_index(json_str, BUFSIZ, i);
				BTK_CHECK_NEG(r, "Could not get JSON string object at index.");

				r = main_fp(opts, (unsigned char *)json_str, strlen(json_str));
				BTK_CHECK_NEG(r, NULL);
			}
		}
	}
	else
	{
		r = main_fp(opts, NULL, 0);
		BTK_CHECK_NEG(r, NULL);
	}

	BTK_CHECK_TRUE(opts->output_format_list && opts->output_format_binary, "Can not use list and binary output formats together.");

	if (opts->output_format_list)
	{
		BTK_CHECK_FALSE(0, "List output format not implemented yet.");
	}
	else if (opts->output_format_binary)
	{
		BTK_CHECK_FALSE(0, "Binary output format not implemented yet.");
	}
	else
	{
		json_print();
	}

	json_free();

	return EXIT_SUCCESS;
}

