#include <stdio.h>
#include "mods/node.h"
#include "mods/message.h"

#define HOST "10.0.0.195"
#define PORT 8333

// Handshake format & protocol
// https://en.bitcoin.it/wiki/Protocol_documentation#version
// https://en.bitcoin.it/wiki/Version_Handshake

int main(void) {
	Node n;
	Message m;

	n = node_connect(HOST, PORT);
	
	printf("Connected on socket: %i\n", node_socket(n));
	
	node_disconnect(n);
	
	m = message_new(MESSAGE_MAGIC_MAINNET, MESSAGE_COMMAND_VERSION);
	
	return 1;
}
