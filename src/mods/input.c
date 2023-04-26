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
#include "mods/json.h"
#include "mods/cJSON/cJSON.h"

static unsigned char *pre_buffer = NULL;
static size_t pre_buffer_len = 0;

int input_get(input_item *input)
{
	ssize_t r;
	ssize_t buffer_size = BUFSIZ;
	size_t read_total = 0;
	size_t i = 1;
	unsigned char *buffer;

	buffer = malloc(buffer_size);
	ERROR_CHECK_NULL(buffer, "Memory allocation error.");

	memset(buffer, 0, buffer_size);

	while ((r = read(STDIN_FILENO, buffer + read_total, buffer_size)) > 0)
	{
		read_total += r;

		if (r == buffer_size)
		{
			i++;

			buffer = realloc(buffer, buffer_size * i);
			ERROR_CHECK_NULL(buffer, "Memory allocation error.");

			memset(buffer + read_total, 0, buffer_size);
		}
	}
	if (r < 0)
	{
		error_log("Input read error. Errno: %i", errno);
		return -1;
	}

	if (pre_buffer)
	{
		buffer = realloc(buffer, read_total + pre_buffer_len);
		ERROR_CHECK_NULL(buffer, "Memory allocation error.");

		memmove(buffer + pre_buffer_len, buffer, read_total);
		memcpy(buffer, pre_buffer, pre_buffer_len);

		read_total += pre_buffer_len;

		free(pre_buffer);
		pre_buffer = NULL;
	}

	if (read_total > 0)
	{
		(*input) = input_new_item(buffer, read_total);
		ERROR_CHECK_NULL((*input), "Could not create new input item.");

		free(buffer);
	}

	return 1;
}

int input_get_line(input_item *input)
{
	ssize_t r;
	size_t i = 0;
	char c;
	unsigned char *buffer;

	buffer = malloc(BUFSIZ);
	ERROR_CHECK_NULL(buffer, "Memory allocation error.");

	memset(buffer, 0, BUFSIZ);

	if (pre_buffer)
	{
		// We assume pre buffer contains a full line.
		if (pre_buffer[pre_buffer_len - 1] == '\n')
		{
			pre_buffer_len--;
		}

		(*input) = input_new_item(pre_buffer, pre_buffer_len);
		ERROR_CHECK_NULL((*input), "Could not create new input item.");

		free(pre_buffer);
		pre_buffer = NULL;

		return 1;
	}

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

	(*input) = input_new_item(buffer, i);
	ERROR_CHECK_NULL((*input), "Could not create new input item.");

	free(buffer);

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

		if (pre_buffer)
		{
			memcpy(buffer, pre_buffer, pre_buffer_len);
			buffer_len = pre_buffer_len;

			free(pre_buffer);
			pre_buffer = NULL;
		}
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

	if (!cJSON_IsArray(jobj) && !cJSON_IsObject(jobj))
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

	r = input_parse_from_json(input, jobj);
	ERROR_CHECK_NEG(r, "Could not parse json input object.");

	json_free(jobj);

	return 1;
}

int input_get_format(void)
{
	ssize_t r;
	unsigned char c;
	int format = 0;

	pre_buffer = malloc(BUFSIZ);
	ERROR_CHECK_NULL(pre_buffer, "Memory alocation error.");

	pre_buffer_len = 0;

	while ((r = read(STDIN_FILENO, &c, 1)) > 0)
	{
		pre_buffer[pre_buffer_len] = c;
		pre_buffer_len++;

		if (c <= 6 || (c >= 14 && c <= 31) || c >= 126)
		{
			format = INPUT_FORMAT_BINARY;
			break;
		}

		if (c == '[' || c == '{')
		{
			format = INPUT_FORMAT_JSON;
			break;
		}

		if (c == '\n')
		{
			format = INPUT_FORMAT_LIST;
			break;
		}
	}
	if (r < 0)
	{
		error_log("Input read error. %s.", strerror(errno));
		return -1;
	}

	if (!format)
	{
		format = INPUT_FORMAT_LIST;
	}

	return format;
}

int input_parse_from_json(input_item *input, cJSON *jobj)
{
	int r;
	static int i = 0;
	static char *(input_list[100]);

	if (cJSON_IsArray(jobj))
	{
		int j = 0;
		cJSON *item;

		if (jobj->string)
		{
			input_list[i++] = jobj->string;
		}

		while ((item = cJSON_GetArrayItem(jobj, j++)) != NULL)
		{
			char string[BUFSIZ];
			input_item new = NULL;
			input_item tmp;
			input_item tmp_head;

			tmp_head = NULL;
			tmp = NULL;

			memset(string, 0, BUFSIZ);

			r = json_input_to_string(string, item);
			ERROR_CHECK_NEG(r, "Could not convert input item to string.");

			new = input_new_item((unsigned char *)string, strlen(string));
			ERROR_CHECK_NULL(new, "Could not create new input item.");

			for (int k = i - 1; k >= 0; k--)
			{
				if (tmp == NULL)
				{
					tmp = input_new_item((unsigned char *)input_list[k], strlen(input_list[k]));
					ERROR_CHECK_NULL(tmp, "Could not create new input item.");

					tmp_head = tmp;
				}
				else
				{
					tmp->input = input_new_item((unsigned char *)input_list[k], strlen(input_list[k]));
					ERROR_CHECK_NULL(tmp->input, "Could not create new input item.");

					tmp = tmp->input;
				}
			}
			new->input = tmp_head;

			(*input) = input_append_item((*input), new);
		}

		if (jobj->string)
		{
			i--;
		}
	}
	else if (cJSON_IsObject(jobj))
	{
		int j = 0;
		cJSON *item;

		if (jobj->string)
		{
			input_list[i++] = jobj->string;
		}

		while ((item = cJSON_GetArrayItem(jobj, j++)) != NULL)
		{
			r = input_parse_from_json(input, item);
			ERROR_CHECK_NEG(r, "Could not traverse json input object.");
		}

		if (jobj->string)
		{
			i--;
		}
	}
	else
	{
		error_log("JSON is not an arry or an object.");
		return -1;
	}

	return 1;
}

input_item input_new_item(unsigned char *data, size_t len)
{
	input_item new = NULL;

	new = malloc(sizeof(*new));
	if (new == NULL)
	{
		error_log("Memory allocation error");
		return NULL;
	}

	new->data = malloc(len);
	if (new->data == NULL)
	{
		error_log("Memory allocation error");
		return NULL;
	}

	memcpy(new->data, data, len);
	new->len = len;
	new->input = NULL;
	new->next = NULL;

	return new;
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

input_item input_copy_item(input_item item)
{
	input_item copy;

	copy = input_new_item(item->data, item->len);
	if (copy == NULL)
	{
		return NULL;
	}

	if (item->input)
	{
		copy->input = input_copy_item(item->input);
	}

	return copy;
}

void input_free(input_item list)
{
	if (list == NULL)
	{
		return;
	}

	input_free(list->input);
	input_free(list->next);
	free(list->data);
	free(list);
}
