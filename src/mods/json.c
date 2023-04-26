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

int json_init_output(cJSON **jobj, int trace_flag)
{
	if (trace_flag)
	{
		*jobj = cJSON_CreateObject();
		ERROR_CHECK_NULL(*jobj, "Could not create JSON trace object.");
	}
	else
	{
		*jobj = cJSON_CreateArray();
		ERROR_CHECK_NULL(*jobj, "Could not create JSON output array.");
	}

	return 1;
}

int json_add_output(cJSON *jobj, char *output, char *input)
{
	cJSON *json_string_output = NULL;
	cJSON *json_array_output = NULL;
	cJSON *input_object = NULL;
	cJSON *output_object = NULL;
	cJSON *output_object_parent = NULL;

	assert(jobj);
	assert(output);

	if (cJSON_GetObjectItem(jobj, output))
	{
		assert(input);

		output_object = cJSON_DetachItemFromObject(jobj, output);
		ERROR_CHECK_NULL(output_object, "Could not detach existing output object.");

		if ((input_object = cJSON_GetObjectItem(jobj, input)))
		{
			cJSON_AddItemToObject(input_object, output, output_object);
		}
		else
		{
			output_object_parent = cJSON_CreateObject();
			ERROR_CHECK_NULL(output_object_parent, "Could not create parent output object.");

			cJSON_AddItemToObject(output_object_parent, output, output_object);

			cJSON_AddItemToObject(jobj, input, output_object_parent);
		}
	}
	else if (input && (json_array_output = cJSON_GetObjectItem(jobj, input)))
	{
		assert(cJSON_IsArray(json_array_output));

		json_string_output = cJSON_CreateString(output);
		ERROR_CHECK_NULL(json_string_output, "Could not generate JSON output string.");

		cJSON_AddItemToArray(json_array_output, json_string_output);
	}
	else if (input)
	{
		assert(*input);
		assert(cJSON_IsObject(jobj));

		json_array_output = cJSON_CreateArray();
		ERROR_CHECK_NULL(json_array_output, "Could not generate JSON output array.");

		json_string_output = cJSON_CreateString(output);
		ERROR_CHECK_NULL(json_string_output, "Could not generate JSON output string.");

		cJSON_AddItemToArray(json_array_output, json_string_output);
		cJSON_AddItemToObject(jobj, input, json_array_output);
	}
	else
	{
		assert(cJSON_IsArray(jobj));

		json_string_output = cJSON_CreateString(output);
		ERROR_CHECK_NULL(json_string_output, "Could not generate JSON output string.");

		cJSON_AddItemToArray(jobj, json_string_output);
	}

	return 1;
}

int json_input_valid(cJSON *jobj)
{
	assert(jobj);

	if (!cJSON_IsArray(jobj))
	{
		return 0;
	}

	return 1;
}

int json_input_next(char *string, cJSON *jobj)
{
	static int i = 0;
	cJSON *tmp;

	assert(jobj);
	assert(cJSON_IsArray(jobj));

	tmp = cJSON_GetArrayItem(jobj, i++);
	if (tmp == NULL)
	{
		// reset index in case of streaming
		i = 0;

		return 0;
	}

	if (cJSON_IsBool(tmp))
	{
		// represent bool as an integer (1 or 0)
		sprintf(string, "%i", tmp->valueint);
	}
	else if (cJSON_IsNumber(tmp))
	{
		if (tmp->valueint == INT_MAX)
		{
			error_log("Integer too large. Wrap large numbers in quotes.");
			return -1;
		}

		sprintf(string, "%i", tmp->valueint);
	}
	else if (cJSON_IsString(tmp))
	{
		strcpy(string, tmp->valuestring);
	}
	else
	{
		error_log("JSON array did not contain bool, string or number at index %d.", i);
		return -1;
	}

	return 1;
}

int json_input_to_string(char *string, cJSON *item)
{
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
		error_log("JSON array contained an object of an unsupported type.");
		return -1;
	}

	return 1;
}

int json_output_size(cJSON *jobj)
{
	assert(jobj);
	assert(cJSON_IsArray(jobj) || cJSON_IsObject(jobj));

	return cJSON_GetArraySize(jobj);
}




int json_init_object(cJSON **jobj)
{
	*jobj = cJSON_CreateObject();
	ERROR_CHECK_NULL(*jobj, "Could not create JSON object.");

	return 1;
}

int json_init_array(cJSON **jobj)
{
	*jobj = cJSON_CreateArray();
	ERROR_CHECK_NULL(*jobj, "Could not create JSON array.");

	return 1;
}

int json_is_object(cJSON *jobj)
{
	if (cJSON_IsObject(jobj))
	{
		return 1;
	}
	else
	{
		return 0;
	}
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

int json_get_key_obj(cJSON **output, cJSON *jobj, char *key)
{
	assert(output);
	assert(jobj);
	assert(key);

	(*output) = cJSON_GetObjectItem(jobj, key);
	if ((*output) == NULL)
	{
		error_log("JSON object does not contain key %s.", key);
		return -1;
	}

	return 1;
}

int json_get_key_string(char **output, cJSON *jobj, char *key)
{
	cJSON *tmp;

	assert(output);
	assert(jobj);
	assert(key);

	tmp = cJSON_GetObjectItem(jobj, key);
	if (tmp == NULL)
	{
		error_log("JSON object does not contain key %s.", key);
		return -1;
	}

	if (!cJSON_IsString(tmp))
	{
		error_log("Object not string at key %s.", key);
		return -1;
	}

	(*output) = malloc(strlen(tmp->valuestring) + 1);
	ERROR_CHECK_NULL((*output), "Memory allocation error.");

	memset((*output), 0, strlen(tmp->valuestring) + 1);

	strcpy((*output), tmp->valuestring);

	return 1;
}

// Future implementations
//int json_get_key_int(int output, cJSON *jobj, char *key)
//int json_get_key_bool(int *output, cJSON *jobj, char *key)

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

int json_get_index_object(cJSON **array_obj, cJSON *jobj, int i, char *key)
{
	cJSON *tmp;
	cJSON *array;

	assert(jobj);

	if (key)
	{
		array = cJSON_GetObjectItem(jobj, key);
		if (array == NULL)
		{
			error_log("JSON object does not contain key %s.", key);
			return -1;
		}
	}
	else
	{
		array = jobj;
	}

	if (!cJSON_IsArray(array))
	{
		error_log("Json object must be an array.");
		return -1;
	}

	tmp = cJSON_GetArrayItem(array, i);
	if (tmp == NULL)
	{
		return 0;
	}

	if (!cJSON_IsObject(tmp))
	{
		error_log("Item at index %i is not a json object.", i);
		return -1;
	}

	(*array_obj) = tmp;

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

int json_delete_key(cJSON *jobj, char *key)
{
	assert(jobj);
	assert(key);

	cJSON_DeleteItemFromObject(jobj, key);

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

int json_from_string(cJSON **jobj, char *string)
{
	assert(string);

	(*jobj) = cJSON_ParseWithOpts(string, NULL, 1);
	ERROR_CHECK_NULL((*jobj), "String contained invalid JSON.");

	return 1;
}

int json_free(cJSON *jobj)
{
	cJSON_Delete(jobj);

	return 1;
}
