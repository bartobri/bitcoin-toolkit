/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stddef.h>
#include "verack.h"


struct Verack {
	// nothing here
};

int verack_new(Verack v) {
	(void)v;

	return 1;
}

size_t verack_sizeof(void)
{
	return sizeof(struct Verack);
}
