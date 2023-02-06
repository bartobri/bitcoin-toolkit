/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "mods/input.h"
#include "mods/error.h"

int input_get(unsigned char **input, size_t *len)
{
	ssize_t r;
	size_t read_total = 0;
	int i = 0;

	(*input) = malloc(BUFSIZ);
	ERROR_CHECK_NULL((*input), "Memory allocation error.");

	while ((r = read(STDIN_FILENO, (*input) + read_total, BUFSIZ)) > 0)
	{
		read_total += r;
		i++;

		if (r == BUFSIZ)
		{
			(*input) = realloc((*input), BUFSIZ * (i + 1));
			ERROR_CHECK_NULL((*input), "Memory allocation error.");
		}
	}

	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	*len = read_total;

	return 1;
}
