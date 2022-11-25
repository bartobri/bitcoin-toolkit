/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mods/json.h"
#include "mods/error.h"
#include "mods/cJSON/cJSON.h"

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