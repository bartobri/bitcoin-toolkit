#ifndef NODE_H
#define NODE_H 1

#include "message.h"

typedef struct Node *Node;

int node_connect(Node, const char *, int);
void node_disconnect(Node);
void node_write_message(Node, Message);
Message node_get_message(Node, char *);
int node_socket(Node);
size_t node_sizeof(void);

#endif
