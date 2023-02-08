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
#include <assert.h>
#include "mods/input.h"
#include "mods/json.h"
#include "mods/qrcode.h"
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

int btk_print_output(output_list, opts_p);

int main(int argc, char *argv[])
{
	int i, r = 0;
	unsigned char *input = NULL; 
	size_t input_len = 0;
	char *command = NULL;
	char command_str[BUFSIZ];
	char json_str[BUFSIZ];
	opts_p opts = NULL;
	char *opts_string = NULL;
	int (*main_fp)(output_list *, opts_p, unsigned char *, size_t) = NULL;
	int (*input_fp)(opts_p) = NULL;
	cJSON *json_input;
	output_list output = NULL;

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

	BTK_CHECK_TRUE((argc <= 1), "Missing command parameter.");

	command = argv[1];

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

	BTK_CHECK_TRUE(opts->input_format_binary && opts->input_format_list, "Can not use both binary and list input format opts.");

	if (input_fp(opts))
	{
		if (opts->input_format_binary)
		{
			r = input_get(&input, &input_len);
			BTK_CHECK_NEG(r, NULL);

			r = main_fp(&output, opts, input, input_len);
			BTK_CHECK_NEG(r, NULL);

			if (opts->output_stream)
			{
				r = btk_print_output(output, opts);
				BTK_CHECK_NEG(r, "Error printing output.");

				output_free(output);
				output = NULL;
			}
		}
		else if (opts->input_format_list)
		{
			while ((r = input_get_line(&input)) > 0)
			{	
				// Ignore empty strings
				if (strlen((char *)input) == 0)
				{
					continue;
				}

				r = main_fp(&output, opts, input, strlen((char *)input));
				BTK_CHECK_NEG(r, NULL);

				if (opts->output_stream)
				{
					r = btk_print_output(output, opts);
					BTK_CHECK_NEG(r, "Error printing output.");

					output_free(output);
					output = NULL;
				}

				free(input);
			}
			BTK_CHECK_NEG(r, NULL);
		}
		else
		{
			while ((r = input_get_json(&json_input)) > 0)
			{
				memset(json_str, 0, BUFSIZ);
				i = 0;

				while(json_get_index(json_str, BUFSIZ, json_input, i++) > 0)
				{
					r = main_fp(&output, opts, (unsigned char *)json_str, strlen(json_str));
					BTK_CHECK_NEG(r, NULL);

					if (opts->output_stream)
					{
						r = btk_print_output(output, opts);
						BTK_CHECK_NEG(r, "Error printing output.");

						output_free(output);
						output = NULL;
					}

					memset(json_str, 0, BUFSIZ);
				}

				json_free(json_input);
			}
			BTK_CHECK_NEG(r, "Error getting json input.");
		}
	}
	else
	{
		r = main_fp(&output, opts, NULL, 0);
		BTK_CHECK_NEG(r, NULL);
	}
	
	if (output)
	{
		r = btk_print_output(output, opts);
		BTK_CHECK_NEG(r, "Error printing output.");

		output_free(output);
	}

	return EXIT_SUCCESS;
}

int btk_print_output(output_list output, opts_p opts)
{
	int r;
	size_t i;
	char qrcode_str[BUFSIZ];
	cJSON *json_output;
	char *json_output_str = NULL;

	assert(output);

	i = 0;
	if (opts->output_format_binary) { i++; }
	if (opts->output_format_list) { i++; }
	if (opts->output_format_qrcode) { i++; }
	ERROR_CHECK_TRUE((i > 1), "Can not use multiple output formats.");

	if (opts->output_format_list)
	{
		while(output)
		{
			printf("%s\n", (char *)(output->content));

			output = output->next;
		}
	}
	else if (opts->output_format_qrcode)
	{
		while(output)
		{
			memset(qrcode_str, 0, BUFSIZ);

			r = qrcode_from_str(qrcode_str, (char *)(output->content));
			ERROR_CHECK_NEG(r, "Can not generate qr code.");

			printf("\n%s\n", qrcode_str);

			output = output->next;
		}
	}
	else if (opts->output_format_binary)
	{
		while(output)
		{
			for (i = 0; i < output->length; i++)
			{
				fputc(((unsigned char *)(output->content))[i], stdout);
			}

			output = output->next;
		}
	}
	else
	{
		r = json_init(&json_output);
		ERROR_CHECK_NEG(r, "Error initializing JSON input.");

		while(output)
		{
			r = json_add(json_output, (char *)(output->content));
			ERROR_CHECK_NEG(r, "Output handling error.");

			output = output->next;
		}

		r = json_to_string(&json_output_str, json_output);
		ERROR_CHECK_NEG(r, "Error converting output to JSON.");

		printf("%s\n", json_output_str);

		free(json_output_str);
		json_free(json_output);
	}

	return 1;
}