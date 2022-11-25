/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef JSON_H
#define JSON_H 1

#include "mods/cJSON/cJSON.h"

int json_init(void);
int json_add(char *);
int json_print(void);
int json_free(void);

#endif
