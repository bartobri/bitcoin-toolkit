#include <stdio.h>
#include "mods/node.h"

#define HOST "10.0.0.195"
#define PORT 8333

int main(void) {
	Node n;

	n = node_connect(HOST, PORT);
	
	printf("Connected on socket: %i\n", node_socket(n));
	
	node_disconnect(n);
	
	return 1;
}
