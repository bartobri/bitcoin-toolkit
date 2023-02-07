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
#include "mods/cJSON/cJSON.h"

int input_get(unsigned char **input, size_t *len)
{
	ssize_t r;
	size_t read_total = 0;
	int i = 0;

	(*input) = malloc(BUFSIZ);
	ERROR_CHECK_NULL((*input), "Memory allocation error.");

	memset((*input), 0, BUFSIZ);

	while ((r = read(STDIN_FILENO, (*input) + read_total, BUFSIZ)) > 0)
	{
		read_total += r;
		i++;

		if (r == BUFSIZ)
		{
			(*input) = realloc((*input), BUFSIZ * (i + 1));
			ERROR_CHECK_NULL((*input), "Memory allocation error.");

			memset((*input) + read_total, 0, BUFSIZ);
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

int input_get_line(unsigned char **input)
{
	ssize_t r;
	int i = 0;
	char c;

	(*input) = malloc(BUFSIZ);
	ERROR_CHECK_NULL((*input), "Memory allocation error.");

	memset((*input), 0, BUFSIZ);

	while ((r = read(STDIN_FILENO, &c, 1)) > 0)
	{
		if (c == '\n')
		{
			break;
		}

		*((*input) + i) = c;
		i++;

		if (i == BUFSIZ)
		{
			error_log("Did not find a new line before buffer filled.");
			return -1;
		}
	}

	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	if (r == 0 && i == 0)
	{
		return 0;
	}

	return 1;
}

int input_get_json(cJSON **jobj)
{
	ssize_t r;
	static char *input = NULL;
	static int input_len = 0;
	int buffer_len = BUFSIZ;
	int json_len = 0;
	const char *parse_end;

	if (input == NULL)
	{
		input = malloc(buffer_len + 1);
		ERROR_CHECK_NULL(input, "Memory allocation error.");

		memset(input, 0, buffer_len + 1);
	}

	r = read(STDIN_FILENO, input + input_len, buffer_len - input_len);
	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	if (input_len == 0 && r == 0)
	{
		free(input);
		input = NULL;

		return 0;
	}

	(*jobj) = cJSON_ParseWithLengthOpts(input, buffer_len - input_len, &parse_end, 0);
    if ((*jobj) == NULL)
    {
        error_log("Invalid JSON.");
        return -1;
    }

    // Advance parse_end past any white spaces
    while (isspace(*parse_end))
    {
    	parse_end++;
    }

    json_len = parse_end - input;

    // shift remaining input back to start of pointer
    memcpy(input, input + json_len, buffer_len - json_len);
    memset(input + (buffer_len - json_len), 0, json_len);

    input_len = strlen(input);

    return 1;
}
