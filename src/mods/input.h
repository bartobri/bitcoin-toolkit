/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef INPUT_H
#define INPUT_H 1

#define INPUT_GET_MODE_ALL  1
#define INPUT_GET_MODE_LINE 2

int input_get(unsigned char **, size_t *);
int input_available(void);
int input_get_old(unsigned char** dest, char *prompt, int);
int input_get_str(char** dest, char *prompt);
int input_get_from_pipe(unsigned char** dest);

#endif