/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "mods/config.h"
#include "mods/output.h"
#include "mods/opts.h"
#include "mods/error.h"

int btk_config_main(output_item *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int r;
	char config_path[BUFSIZ];
	char output_string[BUFSIZ];

	assert(opts);

	(void)input;
	(void)input_len;

	memset(config_path, 0, BUFSIZ);
	memset(output_string, 0, BUFSIZ);

	r = config_get_path(config_path, opts->test);
	ERROR_CHECK_NEG(r, "Could not get config path.");

	r = config_load(config_path);
	ERROR_CHECK_NEG(r, "Could not load config.");

	if (opts->set)
	{
		char *key;
		char *value;

		key = opts->set;

		value = strstr(key, "=");
		ERROR_CHECK_NULL(value, "Invalid set argument. Missing equals sign.");

		*value = '\0';
		value++;

		ERROR_CHECK_FALSE(config_is_valid(key), "Invalid key.");

		r = config_set(key, value);
		ERROR_CHECK_NEG(r, "Could not set config.");

		r = config_write(config_path);
		ERROR_CHECK_NEG(r, "Could not write config.");

		strcpy(output_string, "Success");
	}
	else if (opts->unset)
	{
		ERROR_CHECK_FALSE(config_is_valid(opts->unset), "Invalid key.");

		r = config_unset(opts->unset);
		ERROR_CHECK_NEG(r, "Could not unset config.");

		r = config_write(config_path);
		ERROR_CHECK_NEG(r, "Could not write config.");

		strcpy(output_string, "Success");
	}
	else if (opts->dump)
	{
		r = config_to_string(output_string);
		ERROR_CHECK_NEG(r, "Could not convert json config file to string.");
	}

	*output = output_append_new_copy(*output, output_string, strlen(output_string) + 1);
	ERROR_CHECK_NULL(*output, "Memory allocation error.");

	config_unload();

	return 1;
}

int btk_config_requires_input(opts_p opts)
{
	assert(opts);

	return 0;
}

int btk_config_init(opts_p opts)
{
	int i;

	assert(opts);

	i = 0;
	if (opts->set) { i++; }
	if (opts->unset) { i++; }
	if (opts->dump) { i++; }
	ERROR_CHECK_TRUE(i > 1, "Can not use set and unset together.");
	ERROR_CHECK_TRUE(i == 0, "Must specify a command option.");

	// Force list output format so it prints the json in output obj.
	opts->output_format_list = 1;

	return 1;
}

int btk_config_cleanup(opts_p opts)
{
	assert(opts);

	return 1;
}