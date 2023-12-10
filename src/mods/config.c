/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "error.h"
#include "json.h"
#include "mods/cJSON/cJSON.h"

#define CONFIG_DEFAULT_FILE        ".btk/btk.conf"
#define CONFIG_DEFAULT_FILE_TEST   "/tmp/.btk/btk_test.conf"

static char *(valid_keys[]) = {"rpc-auth", "hostname", "balance-path", "chainstate-path", NULL};
static cJSON *config_json = NULL;

int config_is_valid(char *key)
{
	assert(key);

	for (int i = 0; valid_keys[i] != NULL; i++)
	{
		if (strcmp(key, valid_keys[i]) == 0)
		{
			return 1;
		}
	}

	return 0;
}

int config_get_path(char *config_path, int test_flag)
{
	int r;
	char *sl;
	struct stat sb;

	assert(config_path);

	if (test_flag)
	{
		strcat(config_path, CONFIG_DEFAULT_FILE_TEST);
	}
	else
	{
		strcpy(config_path, getenv("HOME"));
		if (*config_path == 0)
		{
			error_log("Unable to determine home directory.");
			return -1;
		}
		strcat(config_path, "/");
		strcat(config_path, CONFIG_DEFAULT_FILE);
	}

	// Create directory hierarchy if necessary.
	sl = config_path;
	while ((sl = strstr(++sl, "/")))
	{
		*sl = '\0';
		if (stat(config_path, &sb) == -1)
		{
			r = mkdir(config_path, S_IRUSR | S_IWUSR | S_IXUSR);
			ERROR_CHECK_NEG(r, "Could not create config path.");
		}
		*sl = '/';
	}

	return 1;
}

int config_load(char *config_path)
{
	int r;
	int config_file = 0;
	char config_content[BUFSIZ];
	ssize_t bytes;

	assert(config_path);

	memset(config_content, 0, BUFSIZ);

	config_file = open(config_path, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
	ERROR_CHECK_NEG(config_file, "Could not open configuration file.");

	bytes = read(config_file, config_content, BUFSIZ - 1);
	ERROR_CHECK_NEG(bytes, "Could not read config file.");

	if (bytes > 0)
	{
		r = json_from_string(&config_json, config_content);
		ERROR_CHECK_NEG(r, "Could not get json object form config file.");

		ERROR_CHECK_FALSE(json_is_object(config_json), "Invalid json structure in config file.");
	}
	else
	{
		r = json_init_object(&config_json);
		ERROR_CHECK_NEG(r, "Could not initialize config json object.");
	}

	r = close(config_file);
	ERROR_CHECK_NEG(r, "Could not close config file.");

	return 1;
}

int config_set(char *key, char *value)
{
	int r;

	assert(key);
	assert(value);
	assert(config_json);

	while (json_key_exists(config_json, key))
	{
		r = json_delete_key(config_json, key);
		ERROR_CHECK_NEG(r, "Could not remove config key.");
	}

	r = json_add_string(config_json, value, key);
	ERROR_CHECK_NEG(r, "Could not set key/value in json config.");

	return 1;
}

int config_unset(char *key)
{
	int r;

	ERROR_CHECK_FALSE(json_key_exists(config_json, key), "Key is not set.");

	r = json_delete_key(config_json, key);
	ERROR_CHECK_NEG(r, "Could not remove config key.");

	return 1;
}

int config_exists(char *key)
{
	assert(key);
	assert(config_json);

	return json_key_exists(config_json, key);
}

int config_get(char *value, char *key)
{
	int r;
	char *tmp;

	assert(value);
	assert(key);
	assert(config_json);

	r = json_get_key_string(&tmp, config_json, key);
	ERROR_CHECK_NEG(r, "Could not get key value from config.");

	strcpy(value, tmp);

	free(tmp);

	return 1;
}

int config_to_string(char *string)
{
	int r;
	char *tmp;

	assert(string);
	assert(config_json);

	r = json_to_string(&tmp, config_json);
	ERROR_CHECK_NEG(r, "Could not convert json to string.");

	strcpy(string, tmp);

	free(tmp);

	return 1;
}

int config_write(char *config_path)
{
	int r;
	int config_file = 0;
	char *json_string;
	ssize_t bytes;

	assert(config_path);
	assert(config_json);

	r = json_to_string(&json_string, config_json);
	ERROR_CHECK_NEG(r, "Could not convert json to string.");

	config_file = open(config_path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
	ERROR_CHECK_NEG(config_file, "Could not open configuration file.");

	bytes = write(config_file, json_string, strlen(json_string));
	ERROR_CHECK_NEG(bytes, "Could not write config file.");

	r = close(config_file);
	ERROR_CHECK_NEG(r, "Could not close config file.");

	return 1;
}

int config_unload(void)
{
	assert(config_json);

	json_free(config_json);

	return 1;
}