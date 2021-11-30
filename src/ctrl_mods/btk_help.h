/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_HELP_H
#define BTK_HELP_H 1

int btk_help_init(int argc, char *argv[]);
int btk_help_main(void);
int btk_help_cleanup(void);
void btk_help_commands(void);
void btk_help_privkey(void);
void btk_help_pubkey(void);
void btk_help_vanity(void);
void btk_help_node(void);
void btk_help_version(void);

#endif
