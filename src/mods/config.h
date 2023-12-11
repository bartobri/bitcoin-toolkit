/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef CONFIG_H
#define CONFIG_H 1

#define BTK_VERSION_MAJOR      3
#define BTK_VERSION_MINOR      1
#define BTK_VERSION_PATCH      2

int config_is_valid(char *);
int config_get_path(char *, int);
int config_load(char *);
int config_set(char *, char *);
int config_unset(char *);
int config_exists(char *);
int config_get(char *, char *);
int config_to_string(char *);
int config_write(char *);
int config_unload(void);

#endif
