/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_HELP_H
#define BTK_HELP_H 1

#include "mods/output.h"
#include "mods/opts.h"

int btk_help_main(output_item *, opts_p, unsigned char *, size_t);
int btk_help_requires_input(opts_p);
int btk_help_init(opts_p);
int btk_help_cleanup(opts_p);

#endif
