#ifndef NODE_H
#define NODE_H 1

typedef struct Node *Node;

int node_connect(Node, const char *, int);
int node_write(Node, unsigned char *, size_t);
int node_read(Node, unsigned char**);
void node_disconnect(Node);
size_t node_sizeof(void);

#endif
