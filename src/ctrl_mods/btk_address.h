/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_ADDRESS_H
#define BTK_ADDRESS_H 1

#include "mods/output.h"
#include "mods/opts.h"

int btk_address_main(output_item *, opts_p, unsigned char *, size_t);
int btk_address_requires_input(opts_p);
int btk_address_init(opts_p);
int btk_address_cleanup(opts_p);

#endif