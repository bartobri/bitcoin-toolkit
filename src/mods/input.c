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
#include <sys/ioctl.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include "mods/input.h"
#include "mods/error.h"

int input_get(unsigned char **input, size_t *len)
{
	ssize_t r;
	size_t i = 0;
	size_t mem_size = BUFSIZ;
	unsigned char c;

	(*input) = malloc(mem_size);
	if ((*input) == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}

	memset((*input), 0, mem_size);

	while ((r = read(STDIN_FILENO, &c, 1)) > 0)
	{
		(*input)[i++] = c;

		if (i == mem_size)
		{
			mem_size += BUFSIZ;

			(*input) = realloc((*input), mem_size);
			ERROR_CHECK_NULL((*input), "Memory allocation error.");

			memset((*input) + i, 0, BUFSIZ);
		}
	}

	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	*len = i;

	return 1;
}

int input_available(void)
{
	int r, input_len;
	fd_set input_stream;
	struct timeval timeout;
	void *timeout_p;

	FD_ZERO(&input_stream);
	input_len = 0;

	if (isatty(STDIN_FILENO))
	{
		return 0;
	}

	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;
	timeout_p = &timeout;

	FD_SET(STDIN_FILENO, &input_stream);
	r = select(FD_SETSIZE, &input_stream, NULL, NULL, timeout_p);
	if (r < 0)
	{
		error_log("Error while waiting for input data. Errno: %i", errno);
		return -1;
	}
	if (r > 0)
	{	
		r = ioctl(STDIN_FILENO, FIONREAD, &input_len);
		if (r < 0)
		{
			error_log("Could not determine length of input. Errno: %i", errno);
			return -1;
		}

		if (input_len > 0)
		{
			return 1;
		}
	}

	FD_CLR(STDIN_FILENO, &input_stream);

	return 0;
}

int input_get_old(unsigned char** dest, char *prompt, int mode)
{
	int i, r, input_len;
	fd_set input_stream;
	struct timeval timeout;
	void *timeout_p;
	char c;

	FD_ZERO(&input_stream);
	input_len = 0;

	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;

	if (isatty(STDIN_FILENO))
	{
		timeout_p = NULL;
		if (prompt != NULL)
		{
			printf("%s", prompt);
			fflush(stdout);
		}
	}
	else
	{
		timeout_p = &timeout;
	}

	FD_SET(STDIN_FILENO, &input_stream);
	r = select(FD_SETSIZE, &input_stream, NULL, NULL, timeout_p);
	if (r < 0)
	{
		error_log("Error while waiting for input data. Errno: %i", errno);
		return -1;
	}
	if (r > 0)
	{
		r = ioctl(STDIN_FILENO, FIONREAD, &input_len);
		if (r < 0)
		{
			error_log("Could not determine length of input. Errno: %i", errno);
			return -1;
		}
		if (input_len > 0)
		{
			if (mode == INPUT_GET_MODE_LINE)
			{
				input_len = 0;

				*dest = malloc(BUFSIZ);
				if (*dest == NULL)
				{
					error_log("Memory allocation error.");
					return -1;
				}

				memset(*dest, 0, BUFSIZ);

				for (i = 0; i < BUFSIZ - 1; i++)
				{
					r = read(STDIN_FILENO, &c, 1);
					if (r < 0)
					{
						error_log("Input read error. Errno: %i", errno);
						return -1;
					}
					else if (r == 0)
					{
						// EOF
						break;
					}

					(*dest)[i] = c;

					input_len++;

					if (c == '\n')
					{
						break;
					}
				}
			}
			else
			{
				*dest = malloc(input_len);
				if (*dest == NULL)
				{
					error_log("Memory allocation error.");
					return -1;
				}
				r = read(STDIN_FILENO, *dest, input_len);
				if (r < 0)
				{
					error_log("Input read error. Errno: %i", errno);
					return -1;
				}
			}
		}
	}

	FD_CLR(STDIN_FILENO, &input_stream);

	return input_len;
}

int input_get_str(char** dest, char *prompt)
{
	int r, i, input_len;
	unsigned char *input;

	r = input_get_old(&input, prompt, INPUT_GET_MODE_LINE);
	if (r < 0)
	{
		error_log("Could not get input.");
		return -1;
	}

	if (r > 0)
	{
		if (input[r - 1] == '\n')
		{
			--r;
			if (r > 0 && input[r - 1] == '\r')
			{
				--r;
			}
		}
	}

	if (r == 0)
	{
		error_log("No input provided.");
		return -1;
	}

	input_len = r;

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

int input_get_from_pipe(unsigned char** dest)
{
	int r;

	if (isatty(STDIN_FILENO))
	{
		error_log("Input data from a piped or redirected source is required.");
		return -1;
	}

	r = input_get_old(dest, NULL, INPUT_GET_MODE_ALL);
	if (r < 0)
	{
		error_log("Could not get input.");
		return -1;
	}
	if (r == 0)
	{
		error_log("No input provided.");
		return -1;
	}

	return r;
}