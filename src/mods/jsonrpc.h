/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef JSONRPC_H
#define JSONRPC_H 1

int jsonrpc_init(char *, char *, char *);
int jsonrpc_get_blockcount(int *);
int jsonrpc_get_blockhash(char *, int);
int jsonrpc_get_block(char **, char *);
int jsonrpc_get_rawtransaction(char **, char *);

#endif