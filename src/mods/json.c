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
#include <ctype.h>
#include "mods/json.h"
#include "mods/error.h"
#include "mods/cJSON/cJSON.h"

int json_init(cJSON **jobj)
{
    *jobj = cJSON_CreateArray();
    if (*jobj == NULL)
    {
        error_log("Could not create JSON array object.");
        return -1;
    }

    return 1;
}

int json_add(cJSON *jobj, char *string)
{
    cJSON *json_str;

    assert(jobj);
    assert(string);

    json_str = cJSON_CreateString(string);
    if (json_str == NULL)
    {
        error_log("Could not generate JSON string.");
        return -1;
    }
    
    cJSON_AddItemToArray(jobj, json_str);

    return 1;
}

int json_get_index(char *output, size_t output_len, cJSON *jobj, int i)
{
    cJSON *tmp;

    assert(output);
    assert(jobj);

    tmp = cJSON_GetArrayItem(jobj, i);
    if (tmp == NULL)
    {
        return 0;
    }

    if (!cJSON_IsString(tmp))
    {
        error_log("JSON array must contain strings only.");
        return -1;
    }

    output_len--;

    if (strlen(tmp->valuestring) > output_len)
    {
        error_log("JSON value at index %d too large.", i);
        return -1;
    }

    strncpy(output, tmp->valuestring, output_len);

    return 1;
}

int json_to_string(char **output, cJSON *jobj)
{
    *output = cJSON_Print(jobj);
    if (*output == NULL)
    {
        error_log("Could not print JSON string.");
        return -1;
    }

    return 1;
}

int json_free(cJSON *jobj)
{
    cJSON_Delete(jobj);

    return 1;
}
