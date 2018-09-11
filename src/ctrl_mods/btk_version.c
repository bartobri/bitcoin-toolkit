/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "mods/config.h"

int btk_version_main(int argc, char *argv[], unsigned char *input, size_t input_len)
{
	(void)argc;
	(void)argv;
	(void)input;
	(void)input_len;

	printf("Bitcoin Toolkit Version %d.%d.%d\n", BTK_VERSION_MAJOR, BTK_VERSION_MINOR, BTK_VERSION_REVISION);

	return EXIT_SUCCESS;
}