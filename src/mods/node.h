#ifndef NODE_H
#define NODE_H 1

#include "message.h"

typedef struct Node *Node;

Node node_new(const char *, int);
void node_destroy(Node);
void node_send_message(Node, Message);
int node_read_messages(Node);
int node_socket(Node);

#endif
