/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "mods/input.h"
#include "mods/error.h"
#include "mods/cJSON/cJSON.h"

int input_get(input_item *input)
{
	ssize_t r;
	size_t read_total = 0;
	int i = 0;
	unsigned char *buffer;

	buffer = malloc(BUFSIZ);
	ERROR_CHECK_NULL(buffer, "Memory allocation error.");

	memset(buffer, 0, BUFSIZ);

	while ((r = read(STDIN_FILENO, buffer + read_total, BUFSIZ)) > 0)
	{
		read_total += r;
		i++;

		if (r == BUFSIZ)
		{
			buffer = realloc(buffer, BUFSIZ * (i + 1));
			ERROR_CHECK_NULL(buffer, "Memory allocation error.");

			memset(buffer + read_total, 0, BUFSIZ);
		}
	}

	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	if (read_total > 0)
	{
		(*input) = malloc(sizeof(struct input_item));
		ERROR_CHECK_NULL(*input, "Memory allocation error");

		(*input)->data = buffer;
		(*input)->len = read_total;
		(*input)->next = NULL;
	}

	return 1;
}

int input_get_line(input_item *input)
{
	ssize_t r;
	int i = 0;
	char c;
	unsigned char *buffer;

	buffer = malloc(BUFSIZ);
	ERROR_CHECK_NULL(buffer, "Memory allocation error.");

	memset(buffer, 0, BUFSIZ);

	while ((r = read(STDIN_FILENO, &c, 1)) > 0)
	{
		if (c == '\n')
		{
			break;
		}

		*(buffer + i) = c;
		i++;

		if (i == BUFSIZ)
		{
			error_log("Did not find a new line before buffer filled.");
			return -1;
		}
	}

	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	if (r == 0 && i == 0)
	{
		return 0;
	}

	{	
		(*input) = malloc(sizeof(struct input_item));
		ERROR_CHECK_NULL(*input, "Memory allocation error");

		(*input)->data = buffer;
		(*input)->len = i;
		(*input)->next = NULL;
	}

	return 1;
}

int input_get_json(input_item *input)
{
	ssize_t r;
	static char *buffer = NULL;
	static int buffer_len = 0;
	int buffer_max = BUFSIZ;
	int json_len = 0;
	const char *parse_end;
	cJSON *jobj;

	if (buffer == NULL)
	{
		buffer = malloc(buffer_max + 1);
		ERROR_CHECK_NULL(buffer, "Memory allocation error.");

		memset(buffer, 0, buffer_max + 1);
	}

	r = read(STDIN_FILENO, buffer + buffer_len, buffer_max - buffer_len);
	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	if (buffer_len == 0 && r == 0)
	{
		free(buffer);
		buffer = NULL;

		return 0;
	}

	jobj = cJSON_ParseWithLengthOpts(buffer, buffer_max, &parse_end, 0);
    if (jobj == NULL)
    {
        error_log("Invalid JSON.");
        return -1;
    }

    if (!cJSON_IsArray(jobj))
    {
        error_log("Input JSON must be in array format.");
        return -1;
    }

    // Advance parse_end past any white spaces
    while (isspace(*parse_end))
    {
    	parse_end++;
    }

    json_len = parse_end - buffer;

    // shift remaining buffer back to start of pointer
    memcpy(buffer, buffer + json_len, buffer_max - json_len);
    memset(buffer + (buffer_max - json_len), 0, json_len);

    buffer_len = strlen(buffer);

    // Parse json object into input structure.
    if (cJSON_GetArraySize(jobj) > 0)
    {
    	int i = 0;
    	cJSON *item;
    	char string[BUFSIZ];
    	input_item new_item;
    	input_item tail;

    	while ((item = cJSON_GetArrayItem(jobj, i++)) != NULL)
    	{
    		memset(string, 0, BUFSIZ);

		    if (cJSON_IsBool(item))
		    {
		        // represent bool as an integer (1 or 0)
		        sprintf(string, "%i", item->valueint);
		    }
		    else if (cJSON_IsNumber(item))
		    {
		        if (item->valueint == INT_MAX)
		        {
		            error_log("Integer too large. Wrap large numbers in quotes.");
		            return -1;
		        }

		        sprintf(string, "%i", item->valueint);
		    }
		    else if (cJSON_IsString(item))
		    {
		        strcpy(string, item->valuestring);
		    }
		    else
		    {
		        error_log("JSON array contained an object of unknown type at index %i.", i);
		        return -1;
		    }

		    // Build new input item
		    new_item = malloc(sizeof(*new_item));
		    ERROR_CHECK_NULL(new_item, "Memory allocation error");

		    new_item->data = malloc(strlen(string));
			ERROR_CHECK_NULL(new_item->data, "Memory allocation error");

			memcpy(new_item->data, string, strlen(string));
			new_item->len = strlen(string);
			new_item->next = NULL;

			// append new item
			if ((*input) == NULL)
			{
				(*input) = new_item;
				tail = new_item;
			}
			else
			{
				tail->next = new_item;
				tail = tail->next;
			}
		}
	}

	cJSON_Delete(jobj);

    return 1;
}

input_item input_append_item(input_item list, input_item item)
{
	input_item head;

	head = list;

	if (list == NULL)
	{
		return item;
	}

	while (list->next != NULL)
	{
		list = list->next;
	}
	list->next = item;

	return head;
}

void input_free(input_item list)
{
    input_item tmp;

    if (list == NULL)
    {
        return;
    }

    while (list)
    {
        tmp = list->next;
        free(list->data);
        free(list);
        list = tmp;
    }
}