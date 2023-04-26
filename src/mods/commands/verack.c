/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stddef.h>
#include <assert.h>
#include "verack.h"

int verack_new(Verack *verack) {
	(void)verack;

	return 1;
}

int verack_to_raw(unsigned char *payload_send, Verack verack)
{
	assert(payload_send);
	
	(void)verack;

	return 0;
}

void verack_destroy(Verack verack)
{
	(void)verack;
}
