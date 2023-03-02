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
#include <regex.h>
#include <time.h>
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
#include "ctrl_mods/btk_balance.h"
#include "ctrl_mods/btk_version.h"

#define BTK_CHECK_NEG(x, y)         if (x < 0) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_NULL(x, y)        if (x == NULL) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_FALSE(x, y)       if (!x) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_TRUE(x, y)        if (x) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }

#define BTK_INPUT_KEY  "input"
#define BTK_OUTPUT_KEY "output"

static regex_t grep;

int btk_init(opts_p);
int btk_cleanup(opts_p);
int btk_print_output(output_list, opts_p, char *, cJSON *);

int main(int argc, char *argv[])
{
	int i, r = 0;
	unsigned char *input = NULL; 
	size_t input_len = 0;
	char *command = NULL;
	char command_str[BUFSIZ];
	char json_str[BUFSIZ];
	opts_p opts = NULL;
	cJSON *json_input;
	cJSON *tmp;
	output_list output = NULL;

	int (*init_fp)(opts_p) = NULL;
	int (*input_fp)(opts_p) = NULL;
	int (*main_fp)(output_list *, opts_p, unsigned char *, size_t) = NULL;
	int (*cleanup_fp)(opts_p) = NULL;

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
		main_fp = &btk_privkey_main;
		input_fp = &btk_privkey_requires_input;
		init_fp = &btk_privkey_init;
		cleanup_fp = &btk_privkey_cleanup;
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		main_fp = &btk_pubkey_main;
		input_fp = &btk_pubkey_requires_input;
		init_fp = &btk_pubkey_init;
		cleanup_fp = &btk_pubkey_cleanup;
	}
	else if (strcmp(command, "address") == 0)
	{
		main_fp = &btk_address_main;
		input_fp = &btk_address_requires_input;
		init_fp = &btk_address_init;
		cleanup_fp = &btk_address_cleanup;
	}
	else if (strcmp(command, "node") == 0)
	{
		main_fp = &btk_node_main;
		input_fp = &btk_node_requires_input;
		init_fp = &btk_node_init;
		cleanup_fp = &btk_node_cleanup;
	}
	else if (strcmp(command, "balance") == 0)
	{
		main_fp = &btk_balance_main;
		input_fp = &btk_balance_requires_input;
		init_fp = &btk_balance_init;
		cleanup_fp = &btk_balance_cleanup;
	}
	else if (strcmp(command, "version") == 0)
	{
		main_fp = &btk_version_main;
		input_fp = &btk_version_requires_input;
		init_fp = &btk_version_init;
		cleanup_fp = &btk_version_cleanup;
	}
	else if (strcmp(command, "help") == 0)
	{
		main_fp = &btk_help_main;
		input_fp = &btk_help_requires_input;
		init_fp = &btk_help_init;
		cleanup_fp = &btk_help_cleanup;
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
	BTK_CHECK_NULL(opts, "Memory allocation error.");

	r = opts_init(opts, command);
	BTK_CHECK_NEG(r, NULL);

	r = opts_get(opts, argc, argv);
	BTK_CHECK_NEG(r, NULL);

	r = btk_init(opts);
	BTK_CHECK_NEG(r, "Could not initialize btk.");

	r = init_fp(opts);
	BTK_CHECK_NEG(r, "Initialization error.");

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
				r = btk_print_output(output, opts, NULL, NULL);
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
					r = btk_print_output(output, opts, (char *)input, NULL);
					BTK_CHECK_NEG(r, "Error printing output.");

					output_free(output);
					output = NULL;
				}

				free(input);
			}
			BTK_CHECK_NEG(r, NULL);
		}
		else if (opts->input_format_json)
		{
			while ((r = input_get_json(&json_input)) > 0)
			{
				memset(json_str, 0, BUFSIZ);
				i = 0;

				while((r = json_get_index(json_str, BUFSIZ, json_input, i++, BTK_OUTPUT_KEY)) > 0)
				{
					r = main_fp(&output, opts, (unsigned char *)json_str, strlen(json_str));
					BTK_CHECK_NEG(r, NULL);

					if (opts->output_stream)
					{
						tmp = cJSON_Duplicate(json_input, 1);
						BTK_CHECK_NULL(tmp, "Could not duplicate json input.");

						r = json_grep_index(tmp, i - 1, BTK_OUTPUT_KEY);
						BTK_CHECK_NEG(r, "Error grepping input at index.");

						r = btk_print_output(output, opts, NULL, tmp);
						BTK_CHECK_NEG(r, "Error printing output.");

						json_free(tmp);

						output_free(output);
						output = NULL;
					}

					memset(json_str, 0, BUFSIZ);
				}
				BTK_CHECK_NEG(r, "Error reading json input.");

				json_free(json_input);
			}
			BTK_CHECK_NEG(r, "Error getting json input.");
		}
		else
		{
			// In theory, we shouldn't ever get here. If we do, check that
			// we are setting the defaults properly in btk_init()
			BTK_CHECK_NEG(-1, "No input format specified.");
		}
	}
	else
	{
		if (opts->output_stream)
		{
			while (1)
			{
				r = main_fp(&output, opts, NULL, 0);
				BTK_CHECK_NEG(r, NULL);

				r = btk_print_output(output, opts, NULL, 0);
				BTK_CHECK_NEG(r, "Error printing output.");

				output_free(output);
				output = NULL;
			}
		}
		else
		{
			r = main_fp(&output, opts, NULL, 0);
			BTK_CHECK_NEG(r, NULL);
		}
	}
	
	if (output)
	{
		r = btk_print_output(output, opts, NULL, NULL);
		BTK_CHECK_NEG(r, "Error printing output.");

		output_free(output);
	}

	r = cleanup_fp(opts);
	BTK_CHECK_NEG(r, "Cleanup error.");

	r = btk_cleanup(opts);
	BTK_CHECK_NEG(r, "Could not cleanup btk.");

	return EXIT_SUCCESS;
}

int btk_init(opts_p opts)
{
	int r, i;

	// Sanity check the opts

	i = 0;
	if (opts->input_format_binary) { i++; }
	if (opts->input_format_list) { i++; }
	if (opts->input_format_json) { i++; }
	ERROR_CHECK_TRUE((i > 1), "Can not use multiple input formats.");
	if (i == 0)
	{
		opts->input_format_json = 1;
	}

	i = 0;
	if (opts->output_format_binary) { i++; }
	if (opts->output_format_list) { i++; }
	if (opts->output_format_qrcode) { i++; }
	if (opts->output_format_json) { i++; }
	ERROR_CHECK_TRUE((i > 1), "Can not use multiple output formats.");
	if (i == 0)
	{
		opts->output_format_json = 1;
	}

	ERROR_CHECK_TRUE(opts->output_format_binary && opts->output_grep, "Can not grep on binary formatted output.");

	// Compile regex for grep here.

	if (opts->output_grep)
	{
		r = regcomp(&grep, opts->output_grep, REG_ICASE|REG_NOSUB);
		ERROR_CHECK_TRUE(r, "Could not compile regex for grep option.");
	}

	return 1;
}

int btk_cleanup(opts_p opts)
{
	if (opts->output_grep)
	{
		regfree(&grep);
	}

	return 1;
}

int btk_print_output(output_list output, opts_p opts, char *input_str, cJSON *input_json)
{
	int r;
	size_t i;
	char qrcode_str[BUFSIZ];
	cJSON *json_output;
	char *json_output_str = NULL;
	cJSON *tmp = NULL;
	static int stream_count = 0;
	static time_t time_last = 0;
	static time_t time_cur = 0;

	assert(output);

	if (opts->output_format_list)
	{
		while(output)
		{
			if (opts->output_grep)
			{
				r = regexec(&grep, (char *)(output->content), 0, NULL, 0);
				if (r == REG_NOMATCH)
				{
					output = output->next;
					continue;
				}
			}

			printf("%s\n", (char *)(output->content));

			output = output->next;
		}
	}
	else if (opts->output_format_qrcode)
	{
		while(output)
		{
			if (opts->output_grep)
			{
				r = regexec(&grep, (char *)(output->content), 0, NULL, 0);
				if (r == REG_NOMATCH)
				{
					output = output->next;
					continue;
				}
			}

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
	else if (opts->output_format_json)
	{
		r = json_init(&json_output);
		ERROR_CHECK_NEG(r, "Error initializing JSON output.");

		if (input_json)
		{
			r = json_add_object(json_output, input_json, BTK_INPUT_KEY);
			ERROR_CHECK_NEG(r, "Error adding JSON input.");
		}
		else if (input_str)
		{
			r = json_init(&tmp);
			ERROR_CHECK_NEG(r, "Error initializing JSON input string.");

			r = json_append_string(tmp, input_str, BTK_OUTPUT_KEY);
			ERROR_CHECK_NEG(r, "Error adding input string to JSON object.");

			r = json_add_object(json_output, tmp, BTK_INPUT_KEY);
			ERROR_CHECK_NEG(r, "Error adding JSON input.");
		}

		while(output)
		{
			if (opts->output_grep)
			{
				r = regexec(&grep, (char *)(output->content), 0, NULL, 0);
				if (r == REG_NOMATCH)
				{
					output = output->next;
					continue;
				}
			}

			r = json_append_string(json_output, (char *)(output->content), BTK_OUTPUT_KEY);
			ERROR_CHECK_NEG(r, "Output handling error.");

			output = output->next;
		}

		if (opts->output_grep == NULL || json_key_exists(json_output, BTK_OUTPUT_KEY))
		{
			r = json_to_string(&json_output_str, json_output);
			ERROR_CHECK_NEG(r, "Error converting output to JSON.");

			printf("%s\n", json_output_str);
		}

		if (tmp)
		{
			json_free(tmp);
		}

		free(json_output_str);
		json_free(json_output);
	}
	else
	{
		error_log("No output format specified.");
		return -1;
	}

	// Handle stream reporting if option is set.
	if (time_last == 0)
	{
		time_last = time(NULL);
	}
	if (opts->output_stream && opts->stream_report && ++stream_count % opts->stream_report == 0)
	{
		time_cur = time(NULL);
		fprintf(stderr, "Stream count: %i, Seconds: %ld\n", stream_count, time_cur - time_last);
		time_last = time_cur;
	}

	return 1;
}