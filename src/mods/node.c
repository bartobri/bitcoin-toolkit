#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include "node.h"
#include "message.h"
#include "mem.h"
#include "assert.h"

#define MAX_MESSAGE_QUEUE 100

struct Node
{
	int sockfd;
	Message mqueue[MAX_MESSAGE_QUEUE];
};

static int node_read(Node, unsigned char**);
static int node_read_messages(Node);

int node_connect(Node node, const char *host, int port) {
	int i, r, sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	
	assert(node);
	assert(host);
	assert(port);
	
	// Set up a socket in the AF_INET domain (Internet Protocol v4 addresses)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		return -1;
	}
	
	// Get a pointer to 'hostent' containing info about host.
	server = gethostbyname(host);
	if (!server)
	{
		return -1;
	}
	
	// Initializing serv_addr memory footprint to all integer zeros ('\0')
	memset(&serv_addr, 0, sizeof(serv_addr));
	
	// Setting up our serv_addr structure
	serv_addr.sin_family = AF_INET;       // Internet Protocol v4 addresses
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);     // Convert port byte order to 'network byte order'
	
	// Connect to server.
	r = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (r < 0)
	{
		return -1;
	}

	// If we get here, connection succeeded. Set up node struct.
	node->sockfd = sockfd;
	for (i = 0; i < MAX_MESSAGE_QUEUE; ++i) {
		node->mqueue[i] = NULL;
	}
	
	return 1;
}

void node_disconnect(Node node) {
	assert(node);
	assert(node->sockfd);
	
	close(node->sockfd);
}

int node_write(Node node, unsigned char *input, size_t input_len) {
	ssize_t r;

	assert(node);
	assert(node->sockfd);
	assert(input);
	assert(input_len);
	
	r = write(node->sockfd, input, input_len);

	if (r < 0) {
		return -1;
	}

	return 1;
}

Message node_get_message(Node n, char *command) {
	int i, c;
	Message m = NULL;

	c = node_read_messages(n);

	if (c < 0)
	{
		perror("Host Read Error");
		return NULL;
	}

	// Check if desired message is in the queue
	for (i = 0; i < MAX_MESSAGE_QUEUE; ++i) {
		if (n->mqueue[i] == NULL) {
			break;
		}
		if (message_cmp_command(n->mqueue[i], command) == 0) {
			m = n->mqueue[i];
			break;
		}
	}

	// Bump the position of the remaining messages in the queue
	while (++i < MAX_MESSAGE_QUEUE) {
		if (n->mqueue[i]) {
			n->mqueue[i-1] = n->mqueue[i];
			n->mqueue[i] = NULL;
		} else {
			break;
		}
	}

	return m;
}

int node_socket(Node n) {
	assert(n);
	
	return n->sockfd;
}

size_t node_sizeof(void)
{
	return sizeof(struct Node);
}

/*
 * Static functions
 */

static int node_read(Node n, unsigned char** buffer) {
	int r;
	int l = 0;
	
	assert(n);
	assert(buffer);

	// Get length of data waiting to be read
	ioctl(n->sockfd, FIONREAD, &r);

	if (r > 0) {

		// Allocate buffer if null
		if (*buffer == NULL) {
			*buffer = ALLOC(r + 1);
		}

		// read incoming data in to buffer
		l = read(n->sockfd, *buffer, r);
		
		return l;
	}
	
	return 0;
}

static int node_read_messages(Node n) {
	Message m;
	unsigned char *s = NULL;
	int l, i, j, r, c;
	
	assert(n);

	j = 0;
	c = 0;

	l = node_read(n, &s);

	// If read() returns an error, pass it up the call stack
	if (l < 0)
	{
		return l;
	}

	while (l > 0) {
		m = ALLOC(message_sizeof());
		r = message_deserialize(m, s + j, (size_t)l);
		if (r < 0)
		{
			// return negative value here
		}
		j += r;
		l -= j;

		r = message_is_valid(m);
		if (r < 0)
		{
			// return a negative value here
		}

		if (r) {
			for (i = 0; i < MAX_MESSAGE_QUEUE; ++i) {
				if (n->mqueue[i] == NULL) {
					break;
				}
			}
			assert(i < MAX_MESSAGE_QUEUE);
			n->mqueue[i] = m;
			
			++c;
		} else {
			FREE(m);
		}
	}
	return c;
}
