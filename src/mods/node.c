#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include "node.h"
#include "message.h"
#include "mem.h"
#include "assert.h"

#define TIMEOUT 5

struct Node {
	int sockfd;
};

/*
 * Static Function Declarations
 */
static Node node_connect(const char *, int);
static void node_disconnect(Node);
static void node_send(Node, unsigned char *, size_t);
static int node_read(Node, unsigned char**);
static void node_free(Node);

/*
 * External functions
 */
Node node_new(const char *host, int port) {
	assert(host);
	assert(port);

	return node_connect(host, port);
}

void node_destroy(Node n) {
	assert(n);

	node_disconnect(n);
	node_free(n);
}

void node_send_message(Node n, Message m) {
	unsigned char *s;
	size_t l;

	assert(n);
	assert(m);

	l = message_serialize(m, &s);

	node_send(n, s, l);
}

int node_read_messages(Node n) {
	Message m = NULL;
	unsigned char *s = NULL;
	int l, i = 0, c = 0;
	
	assert(n);

	l = node_read(n, &s);
	
	// Read error if l is less than zero
	assert(l >= 0);

	while (l > 0) {
		i += (int)message_deserialize(s + i, &m, (size_t)l);
		l -= i;
		++c;
	}

	return c;
}

int node_socket(Node n) {
	assert(n);
	
	return n->sockfd;
}

/*
 * Static functions
 */
static Node node_connect(const char *host, int port) {
	int t, sockfd = 0;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	Node r;
	
	assert(host);
	assert(port);
	
	// Set up a socket in the AF_INET domain (Internet Protocol v4 addresses)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);
	
	// Get a pointer to 'hostent' containing info about host.
	server = gethostbyname(host);
	assert(server);
	
	// Initializing serv_addr memory footprint to all integer zeros ('\0')
	memset(&serv_addr, 0, sizeof(serv_addr));
	
	// Setting up our serv_addr structure
	serv_addr.sin_family = AF_INET;       // Internet Protocol v4 addresses
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);     // Convert port byte order to 'network byte order'
	
	// Connect to server. Error if can't connect.
	t = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	assert(t >= 0);
	
	NEW(r);

	r->sockfd = sockfd;
	
	return r;
}

static void node_disconnect(Node n) {
	assert(n);
	assert(n->sockfd);
	
	close(n->sockfd);
}

static void node_send(Node n, unsigned char *data, size_t l) {
	ssize_t r;

	assert(n);
	assert(n->sockfd);
	assert(data);
	assert(l);
	
	r = write(n->sockfd, data, l);
	
	// TODO - handle this by reading errno and taking apropriate action
	assert(r > 0);
}

static int node_read(Node n, unsigned char** buffer) {
	int r;
	int timeout = TIMEOUT;
	int l = 0;
	
	assert(n);
	assert(buffer);

	while (timeout--) {
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
	
		sleep(1);
	}
	
	return 0;
}

static void node_free(Node n) {
	FREE(n);
}
