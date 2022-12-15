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
#include "mods/json.h"
#include "mods/error.h"
#include "mods/config.h"

int btk_version_init(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	return 1;
}

int btk_version_main(void)
{
	int r;
	char output_str[BUFSIZ];

	memset(output_str, 0, BUFSIZ);

	json_init();

	sprintf(output_str, "Bitcoin Toolkit Version %d.%d.%d", BTK_VERSION_MAJOR, BTK_VERSION_MINOR, BTK_VERSION_REVISION);

	r = json_add(output_str);
	ERROR_CHECK_NEG(r, "Error while generating JSON.");

	json_print();
	json_free();

	return 1;
}

int btk_version_cleanup(void)
{
	return 1;
}