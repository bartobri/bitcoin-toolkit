/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include "mods/error.h"
#include "mods/cJSON/cJSON.h"

static cJSON *json_obj;

int json_init(void)
{
    json_obj = cJSON_CreateObject();

    return 1;
}

int json_add_string(char *string, char *key)
{
    cJSON *json_obj_str;

    json_obj_str = cJSON_CreateString(string);
    if (json_obj_str == NULL)
    {
        error_log("Could not generate JSON string.");
        return -1;
    }

    cJSON_AddItemToObject(json_obj, key, json_obj_str);

    // TODO - destroy json string oject

    return 1;
}

int json_print(void)
{
    char *json_output;

    json_output = cJSON_Print(json_obj);
    if (json_output == NULL)
    {
        error_log("Could not print JSON string.");
        return -1;
    }

    printf("%s\n", json_output);

    return 1;
}