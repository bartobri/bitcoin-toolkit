/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef CONFIG_H
#define CONFIG_H 1

#define BTK_VERSION_MAJOR      3
#define BTK_VERSION_MINOR      0
#define BTK_VERSION_REVISION   0

int config_get_path(char *);
int config_load(char *);
int config_set(char *, char *);
int config_unset(char *);
int config_to_string(char *);
int config_write(char *);

#endif