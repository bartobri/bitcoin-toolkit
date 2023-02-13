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
#include <limits.h>
#include "mods/json.h"
#include "mods/error.h"
#include "mods/cJSON/cJSON.h"

int json_init(cJSON **jobj)
{
    *jobj = cJSON_CreateObject();
    if (*jobj == NULL)
    {
        error_log("Could not create JSON object.");
        return -1;
    }

    return 1;
}

int json_add_bool(cJSON *jobj, int val, char *key)
{
    cJSON *r;

    assert(jobj);

    r = cJSON_AddBoolToObject(jobj, key, val);
    ERROR_CHECK_NULL(r, "Could not add bool to object.");

    return 1;
}

int json_add_number(cJSON *jobj, double num, char *key)
{
    cJSON *r;

    assert(jobj);

    r = cJSON_AddNumberToObject(jobj, key, num);
    ERROR_CHECK_NULL(r, "Could not add number to object.");

    return 1;
}

int json_add_string(cJSON *jobj, char *str, char *key)
{
    cJSON *r;

    assert(jobj);

    r = cJSON_AddStringToObject(jobj, key, str);
    ERROR_CHECK_NULL(r, "Could not add string to object.");

    return 1;
}

int json_add_object(cJSON *jobj, cJSON *added, char *key)
{
    int r;
    cJSON *tmp;

    assert(jobj);
    assert(added);

    tmp = cJSON_Duplicate(added, 1);
    ERROR_CHECK_NULL(tmp, "Could not duplicate json input.");

    r = cJSON_AddItemToObject(jobj, key, tmp);
    ERROR_CHECK_FALSE(r, "Error adding item to JSON object.");

    return 1;
}

int json_get_index(char *output, size_t output_len, cJSON *jobj, int i, char *key)
{
    cJSON *tmp;
    cJSON *output_arr;

    assert(output);
    assert(jobj);

    if (key)
    {
        output_arr = cJSON_GetObjectItem(jobj, key);
        if (output_arr == NULL)
        {
            error_log("JSON object does not contain key %s.", key);
            return -1;
        }
    }
    else
    {
        output_arr = jobj;
    }

    tmp = cJSON_GetArrayItem(output_arr, i);
    if (tmp == NULL)
    {
        return 0;
    }

    if (cJSON_IsBool(tmp))
    {
        // represent bool as an integer (1 or 0)
        sprintf(output, "%i", tmp->valueint);
    }
    else if (cJSON_IsNumber(tmp))
    {
        if (tmp->valueint == INT_MAX)
        {
            error_log("Integer too large. Wrap large numbers in quotes.");
            return -1;
        }
        sprintf(output, "%i", tmp->valueint);
    }
    else if (cJSON_IsString(tmp))
    {
        if (strlen(tmp->valuestring) > output_len - 1)
        {
            error_log("JSON value at index %d too large.", i);
            return -1;
        }

        strncpy(output, tmp->valuestring, output_len);
    }
    else
    {
        error_log("JSON array did not contain bool, string or number at index %d.", i);
        return -1;
    }

    return 1;
}

int json_append_string(cJSON *jobj, char *string, char *key)
{
    cJSON *json_str;
    cJSON *output_arr;

    assert(jobj);
    assert(string);

    json_str = cJSON_CreateString(string);
    if (json_str == NULL)
    {
        error_log("Could not generate JSON output string.");
        return -1;
    }

    output_arr = cJSON_GetObjectItem(jobj, key);
    if (output_arr == NULL)
    {
        output_arr = cJSON_CreateArray();
        if (output_arr == NULL)
        {
            error_log("Could not generate JSON output array.");
            return -1;
        }

        cJSON_AddItemToObject(jobj, key, output_arr);
    }

    cJSON_AddItemToArray(output_arr, json_str);

    return 1;
}

int json_key_exists(cJSON *jobj, char *key)
{
    assert(jobj);

    cJSON *output_arr;

    output_arr = cJSON_GetObjectItem(jobj, key);
    if (output_arr == NULL)
    {
        return 0;
    }

    return 1;
}

int json_grep_index(cJSON *jobj, int index, char *key)
{
    cJSON *output_arr;
    cJSON *index_obj;
    cJSON *new_arr;

    assert(jobj);

    output_arr = cJSON_GetObjectItem(jobj, key);
    ERROR_CHECK_NULL(output_arr, "JSON does not have a output object.");

    new_arr = cJSON_CreateArray();
    ERROR_CHECK_NULL(new_arr, "Could not create new JSON array.");

    index_obj = cJSON_Duplicate(cJSON_GetArrayItem(output_arr, index), 1);
    ERROR_CHECK_NULL(index_obj, "Could not duplicate JSON item at index.");

    cJSON_AddItemToArray(new_arr, index_obj);

    cJSON_ReplaceItemInObject(jobj, key, new_arr);
    
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
