/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
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

int input_get_str(unsigned char** dest)
{
	int input_len;

	input_len = input_get(dest);

	if (input_len > 0)
	{
		if ((*dest)[input_len - 1] == '\n')
		{
			(*dest)[input_len - 1] = '\0';
			--input_len;
			if ((*dest)[input_len - 1] == '\r')
			{
				(*dest)[input_len - 1] = '\0';
				--input_len;
			}
		}
		else
		{
			*dest = realloc(*dest, input_len + 1);
			if (*dest == NULL)
			{
				error_log("Memory allocation error.");
				return -1;
			}

			(*dest)[input_len] = '\0';
		}
	}

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

	for (i = 0; (o = getchar()) != '\n'; ++i)
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
	}

	return i;
}

int input_get_from_pipe(unsigned char** dest)
{
	int input_len = 0;

	if (!isatty(fileno(stdin)))
	{
		while (1)
		{
			if (ioctl(STDIN_FILENO, FIONREAD, &input_len) >= 0 && input_len > 0)
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