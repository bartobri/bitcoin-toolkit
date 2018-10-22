/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef NODE_H
#define NODE_H 1

typedef struct Node *Node;

int node_connect(Node, const char *, int);
int node_write(Node, unsigned char *, size_t);
int node_read(Node, unsigned char**);
void node_disconnect(Node);
size_t node_sizeof(void);

#endif
