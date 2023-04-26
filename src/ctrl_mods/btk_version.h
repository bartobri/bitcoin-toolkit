/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_VERSION_H
#define BTK_VERSION_H 1

#include "mods/output.h"
#include "mods/opts.h"

int btk_version_main(output_item *, opts_p, unsigned char *, size_t);
int btk_version_requires_input(opts_p);
int btk_version_init(opts_p);
int btk_version_cleanup(opts_p);

#endif
