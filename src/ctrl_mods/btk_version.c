/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "mods/opts.h"
#include "mods/output.h"
#include "mods/error.h"
#include "mods/config.h"

int btk_version_main(output_item *output, opts_p opts, unsigned char *input, size_t input_len)
{
	char output_str[BUFSIZ];

	(void)opts;
	(void)input;
	(void)input_len;

	memset(output_str, 0, BUFSIZ);

	sprintf(output_str, "Bitcoin Toolkit Version %d.%d.%d", BTK_VERSION_MAJOR, BTK_VERSION_MINOR, BTK_VERSION_PATCH);

	*output = output_append_new_copy(*output, output_str, strlen(output_str) + 1);
	ERROR_CHECK_NULL(*output, "Memory allocation error.");

	return 1;
}

int btk_version_requires_input(opts_p opts)
{
	assert(opts);

	return 0;
}

int btk_version_init(opts_p opts)
{
	assert(opts);

	return 1;
}

int btk_version_cleanup(opts_p opts)
{
	assert(opts);
	
	return 1;
}