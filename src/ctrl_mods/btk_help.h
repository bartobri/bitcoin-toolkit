/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_HELP_H
#define BTK_HELP_H 1

#include "mods/opts.h"

int btk_help_main(opts_p, unsigned char *, size_t);
int btk_help_requires_input(opts_p);

#endif
