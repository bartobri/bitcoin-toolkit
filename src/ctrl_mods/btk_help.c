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
void btk_help_utxodb(void);

int btk_help_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
	char command_str[BUFSIZ];

	assert(opts);

	(void)input;
	(void)input_len;
	(void)output;

	memset(command_str, 0, BUFSIZ);

	strcpy(command_str, "man btk");

	if (opts->subcommand)
	{
		strcat(command_str, "-");
		strcat(command_str, opts->subcommand);
	}

	system(command_str);
	
	return 1;
}

int btk_help_requires_input(opts_p opts)
{
    assert(opts);

    return 0;
}
