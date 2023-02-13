/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef JSON_H
#define JSON_H 1

#include <stddef.h>
#include "mods/cJSON/cJSON.h"

int json_init(cJSON **);
int json_add_bool(cJSON *, int, char *);
int json_add_number(cJSON *, double, char *);
int json_add_string(cJSON *, char *, char *);
int json_add_object(cJSON *, cJSON *, char *);
int json_get_index(char *, size_t, cJSON *, int, char *);
int json_append_string(cJSON *, char *, char *);
int json_key_exists(cJSON *jobj, char *);

int json_grep_output_index(cJSON *, int);
int json_to_string(char **, cJSON *);
int json_free(cJSON *);

#endif
