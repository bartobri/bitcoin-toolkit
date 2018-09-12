/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "mods/input.h"
#include "mods/mem.h"

#define INPUT_INCREMENT 100
#define INPUT_MAX       20000000

size_t input_get(unsigned char** dest)
{
	size_t input_len = 0;

	input_len = input_get_from_pipe(dest);

	if (input_len == 0)
	{
		input_len = input_get_from_keyboard(dest);
	}

	return input_len;
}

size_t input_get_from_keyboard(unsigned char** dest)
{
	size_t i, s = INPUT_INCREMENT;
	int o;

	*dest = ALLOC(s);

	for (i = 0; i < INPUT_MAX && (o = getchar()) != '\n'; ++i)
	{
		if (i == s)
		{
			s += INPUT_INCREMENT;
			RESIZE(*dest, s);
		}
		(*dest)[i] = (unsigned char)o;
	}

	return i;
}

size_t input_get_from_pipe(unsigned char** dest)
{
	size_t input_len = 0;

	if (!isatty(fileno(stdin)))
	{
		while (1)
		{
			if (ioctl(STDIN_FILENO, FIONREAD, &input_len) >= 0 && input_len > 0)
			{
				*dest = ALLOC(input_len);
				read(STDIN_FILENO, *dest, input_len);
				break;
			}
		}
	}

	return input_len;
}