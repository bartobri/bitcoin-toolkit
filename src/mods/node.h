#ifndef NODE_H
#define NODE_H 1

typedef struct Node *Node;

Node node_connect(const char *, int);
int  node_socket(Node);
void node_disconnect(Node);
void node_send(Node, unsigned char *, size_t);
size_t node_read(Node, unsigned char*);

#endif
