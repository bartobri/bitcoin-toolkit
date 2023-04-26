/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef INPUT_H
#define INPUT_H 1

#include "mods/cJSON/cJSON.h"

#define INPUT_FORMAT_BINARY 1
#define INPUT_FORMAT_LIST   2
#define INPUT_FORMAT_JSON   3

typedef struct input_item *input_item;
struct input_item {
	unsigned char *data;
	size_t len;
	input_item input;
	input_item next;
};

int input_get(input_item *);
int input_get_line(input_item *);
int input_get_json(input_item *);
int input_get_format(void);
int input_parse_from_json(input_item *, cJSON *);
input_item input_new_item(unsigned char *, size_t);
input_item input_copy_item(input_item);
input_item input_append_item(input_item, input_item);
void input_free(input_item);

#endif