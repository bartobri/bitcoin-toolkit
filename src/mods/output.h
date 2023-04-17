/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef OUTPUT_H
#define OUTPUT_H 1

#include "input.h"

typedef struct output_item *output_item;
struct output_item {
	void *content;
	size_t length;
	input_item input;
	output_item next;
};

output_item output_new(void *, size_t);
output_item output_append(output_item, output_item);
output_item output_append_new(output_item, void *, size_t);
output_item output_append_new_copy(output_item, void *, size_t);
int output_append_input(output_item, input_item, int);
size_t output_size(output_item);
size_t output_length(output_item);
void output_free(output_item);

#endif