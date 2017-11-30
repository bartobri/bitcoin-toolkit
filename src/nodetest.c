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
	size_t l;
	Node n;
	Message m;
	Version v;
	unsigned char *s;

	n = node_new(HOST, PORT);
	printf("Connected on socket: %i\n", node_socket(n));
	
	v = version_new();
	l = version_serialize(v, &s);
	version_free(v);
	
	m = message_new(VERSION_COMMAND, s, l);
	FREE(s);

	node_send_message(n, m);
	message_free(m);
	
	/*
	printf("Version Message:\n");
	for (i = 0; i < l; ++i) {
		printf("%02x", s[i]);
	}
	printf("\n");
	*/
	
	/*
	printf("Sending Version Message\n");
	node_send(n, s, l);
	*/

	m = node_read_message(n);

	if (m) {
		printf("Received Message Response\n");
	} else {
		printf("No Response from Node\n");
	}

	message_free(m);
	node_destroy(n);
	
	return 1;
}
