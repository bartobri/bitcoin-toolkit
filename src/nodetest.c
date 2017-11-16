#include <stdio.h>
#include "mods/node.h"
#include "mods/message.h"
#include "mods/mem.h"

#define HOST "10.0.0.195"
#define PORT 8333

// Handshake format & protocol
// https://bitcoin.org/en/developer-reference#version
// https://en.bitcoin.it/wiki/Version_Handshake

int main(void) {
	size_t i, l;
	Node n;
	Message m;
	unsigned char *s;

	n = node_connect(HOST, PORT);
	printf("Connected on socket: %i\n", node_socket(n));
	
	m = message_new("version");
	l = message_serialize(m, &s);
	
	printf("Sending Version Message\n");
	node_send(n, s, l);
	
	FREE(s);
	FREE(m);
	s = NULL;
	m = NULL;

	l = node_read(n, &s, 5);
	
	m = message_from_raw(s, l);

	node_disconnect(n);
	FREE(n);
	
	printf("Message Response:\n");
	for (i = 0; i < l; ++i) {
		printf("%02x", s[i]);
	}
	printf("\n");
	
	return 1;
}
