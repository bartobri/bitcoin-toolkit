/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "error.h"

#define RANDOM_SOURCE "/dev/urandom"

int random_get(unsigned char *output, size_t bytes)
{
	size_t i;
	int c;
	FILE *source;

	assert(output);
	assert(bytes);

	source = fopen(RANDOM_SOURCE, "r");
	if (source == NULL)
	{
		error_log("Unable to open source file %s. Errno %i.", RANDOM_SOURCE, errno);
		return -1;
	}

	for (i = 0; i < bytes; ++i)
	{
		c = fgetc(source);
		if (c == EOF)
		{
			error_log("Could not read character from source file %s.", RANDOM_SOURCE);
			return -1;
		}

		output[i] = c;
	}

	fclose(source);

	return 1;
}
