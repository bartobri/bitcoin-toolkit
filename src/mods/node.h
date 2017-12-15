#ifndef NODE_H
#define NODE_H 1

#include "message.h"

typedef struct Node *Node;

Node node_new(const char *, int);
void node_destroy(Node);
void node_write_message(Node, Message);
Message node_get_message(Node, char *);
int node_socket(Node);

#endif
