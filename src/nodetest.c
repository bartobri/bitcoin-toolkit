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
	int i;
	Node n;
	Message m;
	unsigned char *s;

	n = node_new(HOST, PORT);
	printf("Connected on socket: %i\n", node_socket(n));
	
	l = version_new_serialize(&s);

	printf("Version Message:\n");
	for (i = 0; i < (int)l; ++i) {
		printf("%02x", s[i]);
	}
	printf("\n");
	
	m = message_new(VERSION_COMMAND, s, l);
	FREE(s);

	node_write_message(n, m);
	message_free(m);

	i = node_read_messages(n);
	printf("%i messages read\n", i);

	/*
	if (m) {
		printf("Received Message Response\n");
		l = message_serialize(m, &s);
		printf("Response Message:\n");
		for (i = 0; i < l; ++i) {
			printf("%02x", s[i]);
		}
		printf("\n");
		message_free(m);
	} else {
		printf("No Response from Node\n");
	}
	*/

	node_destroy(n);
	
	return 1;
}
