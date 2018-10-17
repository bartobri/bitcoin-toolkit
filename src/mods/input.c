/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "input.h"
#include "error.h"

#define INPUT_INCREMENT 100

int input_get(unsigned char** dest)
{
	int input_len;

	input_len = input_get_from_pipe(dest);

	if (input_len == 0)
	{
		input_len = input_get_from_keyboard(dest);
	}

	return input_len;
}

int input_get_str(char** dest)
{
	int i, input_len;
	unsigned char *input;

	input_len = input_get(&input);

	if (input_len > 0)
	{
		if (input[input_len - 1] == '\n')
		{
			--input_len;
			if (input_len > 0 && input[input_len - 1] == '\r')
			{
				--input_len;
			}
		}
	}

	if (input_len == 0)
	{
		error_log("No input provided.");
		return -1;
	}

	*dest = malloc(input_len + 1);
	if (*dest == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	memset(*dest, 0, input_len + 1);

	for (i = 0; i < input_len; ++i)
	{
		if (isascii(input[i]))
		{
			(*dest)[i] = input[i];
		}
		else
		{
			error_log("Input contains non-ascii characters.");
			return -1;
		}
	}

	free(input);

	return input_len;
}

int input_get_from_keyboard(unsigned char** dest)
{
	int i, o, s;

	s = INPUT_INCREMENT;

	*dest = malloc(s);
	if (*dest == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	for (i = 0; (o = getchar()) != EOF; ++i)
	{
		if (i == s)
		{
			s += INPUT_INCREMENT;
			*dest = realloc(*dest, s);
			if (*dest == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}
		}
		
		(*dest)[i] = (unsigned char)o;

		if (o == '\n')
		{
			break;
		}
	}

	return i;
}

int input_get_from_pipe(unsigned char** dest)
{
	int r, input_len;

	input_len = 0;

	if (!isatty(fileno(stdin)))
	{
		while (1)
		{
			r = ioctl(STDIN_FILENO, FIONREAD, &input_len);
			if (r < 0)
			{
				error_log("Input read error. Errno: %i", errno);
				return -1;
			}
			if (input_len > 0)
			{
				*dest = malloc(input_len);
				if (*dest == NULL)
				{
					error_log("Memory allocation error.");
					return -1;
				}
				read(STDIN_FILENO, *dest, input_len);
				break;
			}
		}
	}

	return input_len;
}