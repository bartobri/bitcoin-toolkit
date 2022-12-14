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

int json_from_input(unsigned char **input, size_t *input_len)
{
    int r;
    size_t i;
    char str[BUFSIZ];

    memset(str, 0, BUFSIZ);

    for (i = 0; i < *input_len; i++)
    {
        if (i > BUFSIZ-1)
        {
            error_log("Input string too large. Consider using -b for arbitrarily large amounts of input data.");
            return -1;
        }
        if (isascii((*input)[i]))
        {
            str[i] = (*input)[i];
        }
        else
        {
            error_log("Input contains non-ascii characters.");
            return -1;
        }
    }

    while (str[(*input_len)-1] == '\n' || str[(*input_len)-1] == '\r')
    {
        str[(*input_len)-1] = '\0';
        (*input_len)--;
    }

    // Free input here because json_from_string reallocates it.
    free(*input);

    r = json_from_string((char **)input, str);
    ERROR_CHECK_NEG(r, "Could not convert input to JSON.");

    *input_len = strlen((char *)*input);

    return 1;
}

int json_from_string(char ** output, char *input)
{
    char *tok;
    cJSON *arr;
    cJSON *str;

    assert (input);

    arr = cJSON_CreateArray();
    if (arr == NULL)
    {
        error_log("Could not create JSON array object.");
        return -1;
    }

    tok = strtok(input, "\n");
    while (tok != NULL)
    {
        str = cJSON_CreateString(tok);
        if (str == NULL)
        {
            error_log("Could not generate JSON string.");
            return -1;
        }
        
        cJSON_AddItemToArray(arr, str);

        tok = strtok(NULL, "\n");
    }

    *output = cJSON_Print(arr);
    if (*output == NULL)
    {
        error_log("Could not print JSON string.");
        return -1;
    }

    cJSON_Delete(arr);

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

    if (cJSON_IsNumber(tmp))
    {
        return 0;
    }

    cJSON_Delete(tmp);

    return 1;
}

int json_set_input(char *input)
{
    int i, len;
    char str[BUFSIZ];
    cJSON *item;
    cJSON *new_item;

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

    len = cJSON_GetArraySize(json_input);
    for (i = 0; i < len; i++)
    {
        item = cJSON_GetArrayItem(json_input, i);
        if (cJSON_IsNumber(item))
        {
            memset(str, 0, BUFSIZ);
            snprintf(str, BUFSIZ-1, "%d", item->valueint);
            new_item = cJSON_CreateString(str);
            if (new_item == NULL)
            {
                error_log("Could not convert number to JSON string.");
                return -1;
            }
            cJSON_ReplaceItemInArray(json_input, i, new_item);
        }
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

int json_get_input_index(char *dest, size_t max_len, int i)
{
    assert(dest);
    assert(max_len > 0);
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

    // Decrement to account for final null byte.
    max_len--;

    if (strlen(string->valuestring) > max_len)
    {
        error_log("JSON value at index %d too large.", i);
        return -1;
    }

    strncpy(dest, string->valuestring, max_len);

    return 1;
}

int json_add(char *string)
{
    cJSON *json_str;

    assert(string);

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

int json_print_input(void)
{
    char *json_output;

    json_output = cJSON_Print(json_input);
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