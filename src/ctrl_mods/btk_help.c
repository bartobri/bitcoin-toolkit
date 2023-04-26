/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "btk_help.h"
#include "mods/output.h"
#include "mods/opts.h"
#include "mods/error.h"

void btk_help_commands(void);
void btk_help_privkey(void);
void btk_help_pubkey(void);
void btk_help_address(void);
void btk_help_node(void);
void btk_help_version(void);

int btk_help_main(output_item *output, opts_p opts, unsigned char *input, size_t input_len)
{
	char command_str[BUFSIZ];
	static int input_index = 0;

	assert(opts);

	(void)input;
	(void)input_len;
	(void)output;

	memset(command_str, 0, BUFSIZ);

	strcpy(command_str, "man btk");

	if (input_index < opts->input_count)
	{
		strcat(command_str, "-");
		strcat(command_str, opts->input[input_index]);

		input_index++;
	}

	system(command_str);

	// Right now, btk expects every command to produce output.
	// So we add a dummy output item here until the code is improved.
	*output = output_append_new_copy(*output, "", 0);
	ERROR_CHECK_NULL(*output, "Memory allocation error.");
	
	return 1;
}

int btk_help_requires_input(opts_p opts)
{
	assert(opts);

	if (opts->input_count > 0)
	{
		return 1;
	}

	return 0;
}

int btk_help_init(opts_p opts)
{
	assert(opts);

	opts->output_format_binary = 1;

	return 1;
}

int btk_help_cleanup(opts_p opts)
{
	assert(opts);
	
	return 1;
}