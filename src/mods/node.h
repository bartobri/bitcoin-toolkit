#ifndef NODE_H
#define NODE_H 1

typedef struct Node *Node;

Node node_connect(const char *, int);
int  node_socket(Node);
void node_disconnect(Node);

#endif
