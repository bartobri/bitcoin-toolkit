#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mods/node.h"
#include "mods/message.h"
#include "mods/mem.h"
#include "mods/commands/version.h"
#include "mods/commands/verack.h"

#define HOST "10.0.0.195"
//#define HOST "68.37.93.83"
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
	
	for (i = 0; i < 5; ++i) {
		if ((m = node_get_message(n, VERSION_COMMAND))) {
			break;
		}
		sleep(1);
	}
	
	if (m) {
		printf("Received Version Message\n");
		l = message_serialize(m, &s);
		for (i = 0; i < (int)l; ++i) {
			printf("%02x", s[i]);
		}
		printf("\n");
		message_free(m);
	} else {
		printf("No version message\n");
	}

	for (i = 0; i < 5; ++i) {
		if ((m = node_get_message(n, VERACK_COMMAND))) {
			break;
		}
		sleep(1);
	}

	if (m) {
		printf("Received Verack Message\n");
		l = message_serialize(m, &s);
		for (i = 0; i < (int)l; ++i) {
			printf("%02x", s[i]);
		}
		printf("\n");
		message_free(m);
	} else {
		printf("No verack message\n");
	}

	node_destroy(n);
	
	return 1;
}
