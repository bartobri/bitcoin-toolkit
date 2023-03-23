/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_CONFIG_H
#define BTK_CONFIG_H 1

#include "mods/output.h"
#include "mods/opts.h"

int btk_config_main(output_item *, opts_p, unsigned char *, size_t);
int btk_config_requires_input(opts_p);
int btk_config_init(opts_p);
int btk_config_cleanup(opts_p);

#endif