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
#include "mods/opts.h"
#include "mods/json.h"
#include "mods/error.h"
#include "mods/config.h"

int btk_version_main(opts_p opts, unsigned char *input, size_t input_len)
{
	int r;
	char output_str[BUFSIZ];

	(void)opts;
	(void)input;
	(void)input_len;

	memset(output_str, 0, BUFSIZ);

	sprintf(output_str, "Bitcoin Toolkit Version %d.%d.%d", BTK_VERSION_MAJOR, BTK_VERSION_MINOR, BTK_VERSION_REVISION);

	r = json_add(output_str);
	ERROR_CHECK_NEG(r, "Error while generating JSON.");

	return 1;
}