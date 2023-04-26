/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef NODE_H
#define NODE_H 1

typedef struct Node *Node;

int node_new(Node *, char *, char *);
int node_connect(Node);
int node_write(Node, unsigned char *, size_t);
int node_read(Node, unsigned char**);
int node_read_message(unsigned char **, Node);
void node_disconnect(Node);
void node_destroy(Node);

#endif
