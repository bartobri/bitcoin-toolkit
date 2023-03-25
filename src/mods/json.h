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

int json_init_output(cJSON **, int);
int json_add_output(cJSON *, char *, char *);
int json_input_valid(cJSON *);
int json_input_next(char *, cJSON *);
int json_input_to_string(char *, cJSON *);
int json_output_size(cJSON *);

int json_init_object(cJSON **);
int json_init_array(cJSON **);
int json_is_object(cJSON *);
int json_add_bool(cJSON *, int, char *);
int json_add_number(cJSON *, double, char *);
int json_add_string(cJSON *, char *, char *);
int json_add_object(cJSON *, cJSON *, char *);
int json_get_key_obj(cJSON **, cJSON *, char *);
int json_get_key_string(char **, cJSON *, char *);
int json_get_index(char *, size_t, cJSON *, int, char *);
int json_get_index_object(cJSON **, cJSON *, int, char *);
int json_append_string(cJSON *, char *, char *);
int json_key_exists(cJSON *jobj, char *);
int json_delete_key(cJSON *, char *);
int json_grep_index(cJSON *, int, char *);
int json_to_string(char **, cJSON *);
int json_from_string(cJSON **, char *);
int json_free(cJSON *);

#endif
