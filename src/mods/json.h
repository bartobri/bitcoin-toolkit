/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef JSON_H
#define JSON_H 1

#include "mods/cJSON/cJSON.h"

int json_init(cJSON **, char *, size_t);
int json_add(cJSON *, char *);
int json_get_index(char *, size_t, cJSON *, int);
int json_print(char **, cJSON *);
int json_free(cJSON *);

#endif
