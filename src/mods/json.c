/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mods/json.h"
#include "mods/error.h"
#include "mods/cJSON/cJSON.h"

static cJSON *json_input;
static cJSON *json_arr;

int json_init(void)
{
    json_arr = cJSON_CreateArray();
    if (json_arr == NULL)
    {
        error_log("Could not create JSON array object.");
        return -1;
    }

    return 1;
}

int json_is_valid(char *string, size_t len)
{
    assert(string);

    cJSON *tmp;

    tmp = cJSON_ParseWithLength(string, len);
    if (tmp == NULL)
    {
        return 0;
    }

    cJSON_Delete(tmp);

    return 1;
}

int json_set_input(char *input)
{
    assert (input);

    json_input = cJSON_Parse(input);
    if (json_input == NULL)
    {
        error_log("Invalid JSON.");
        return -1;
    }

    if (!cJSON_IsArray(json_input))
    {
        error_log("JSON must be an array.");
        return -1;
    }

    return 1;
}

int json_get_input_len(int *len)
{
    assert(len);
    assert(json_input);

    *len = cJSON_GetArraySize(json_input);

    return 1;
}

int json_get_input_index(char *dest, int i)
{
    assert(dest);
    assert(json_input);

    cJSON *string;

    string = cJSON_GetArrayItem(json_input, i);
    if (string == NULL)
    {
        error_log("Index %d does not exist.", i);
        return -1;
    }

    if (!cJSON_IsString(string))
    {
        error_log("JSON array must contain strings only.");
        return -1;
    }

    strcpy(dest, string->valuestring);

    return 1;
}

int json_add(char *string)
{
    cJSON *json_str;

    json_str = cJSON_CreateString(string);
    if (json_str == NULL)
    {
        error_log("Could not generate JSON string.");
        return -1;
    }
    
    cJSON_AddItemToArray(json_arr, json_str);

    return 1;
}

int json_print(void)
{
    char *json_output;

    json_output = cJSON_Print(json_arr);
    if (json_output == NULL)
    {
        error_log("Could not print JSON string.");
        return -1;
    }

    printf("%s\n", json_output);

    free(json_output);

    return 1;
}

int json_free(void)
{
    cJSON_Delete(json_arr);

    return 1;
}