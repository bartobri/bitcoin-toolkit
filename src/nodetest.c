#include <stdio.h>
#include "mods/node.h"
#include "mods/message.h"
#include "mods/mem.h"
#include "mods/commands/version.h"

#define HOST "10.0.0.195"
#define PORT 8333

// Handshake format & protocol
// https://bitcoin.org/en/developer-reference#version
// https://en.bitcoin.it/wiki/Version_Handshake

int main(void) {
	size_t i, l;
	Node n;
	Message m;
	Version v;
	unsigned char *s;

	n = node_connect(HOST, PORT);
	printf("Connected on socket: %i\n", node_socket(n));
	
	v = version_new();
	l = version_serialize(v, &s);
	version_free(v);
	
	m = message_new(MESSAGE_COMMAND_VERSION, s, l);
	FREE(s);
	l = message_serialize(m, &s);
	message_free(m);
	
	printf("Version Message:\n");
	for (i = 0; i < l; ++i) {
		printf("%02x", s[i]);
	}
	printf("\n");
	
	printf("Sending Version Message\n");
	node_send(n, s, l);
	
	FREE(s);
	s = NULL;
	m = NULL;

	l = node_read(n, &s);
	
	if (l) {
		m = message_deserialize(s, l);
		
		printf("Message Response:\n");
		for (i = 0; i < l; ++i) {
			printf("%02x", s[i]);
		}
		printf("\n");
	} else {
		printf("Node timeout before response\n");
	}

	node_disconnect(n);
	node_free(n);
	
	return 1;
}
