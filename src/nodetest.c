#include <stdio.h>
#include "mods/node.h"
#include "mods/message.h"

#define HOST "10.0.0.195"
#define PORT 8333

// Handshake format & protocol
// https://bitcoin.org/en/developer-reference#version
// https://en.bitcoin.it/wiki/Version_Handshake

int main(void) {
	size_t i;
	//Node n;
	Message m;
	unsigned char *s;
	size_t len;

	//n = node_connect(HOST, PORT);
	//printf("Connected on socket: %i\n", node_socket(n));
	//node_disconnect(n);
	
	m = message_new("version");
	len = message_serialize(m, &s);
	
	printf("Version Serialized:\n");
	for (i = 0; i < len; ++i) {
		printf("%02x", s[i]);
	}
	printf("\n");
	
	return 1;
}
