/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
#include "node.h"
#include "error.h"

#define MAX_MESSAGE_QUEUE 100

struct Node
{
	int sockfd;
};

int node_connect(Node node, const char *host, int port)
{
	int r, sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	
	assert(node);
	assert(host);
	assert(port);
	
	// Set up a socket in the AF_INET domain (Internet Protocol v4 addresses)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error_log("Unable to create new socket. Errno %i.", errno);
		return -1;
	}
	
	// Get a pointer to 'hostent' containing info about host.
	server = gethostbyname(host);
	if (!server)
	{
		error_log("Unable to lookup host %s. Errno %i.", host, h_errno);
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
		error_log("Unable to connect to host %s. Errno %i.", host, errno);
		return -1;
	}

	// If we get here, connection succeeded. Set up node struct.
	node->sockfd = sockfd;
	
	return 1;
}

int node_write(Node node, unsigned char *input, size_t input_len)
{
	ssize_t r;

	assert(node);
	assert(node->sockfd);
	assert(input);
	assert(input_len);
	
	r = write(node->sockfd, input, input_len);

	if (r < 0)
	{
		error_log("Unable to write message to node. Errno %i.", errno);
		return -1;
	}

	return 1;
}

int node_read(Node node, unsigned char** buffer)
{
	int r, i;
	int read_total = 0;
	
	assert(node);
	assert(buffer);

	*buffer = malloc(BUFSIZ);
	ERROR_CHECK_NULL((*buffer), "Memory allocation error.");

	memset((*buffer), 0, BUFSIZ);

	while ((r = read(node->sockfd, (*buffer) + read_total, BUFSIZ)) > 0)
	{
		read_total += r;
		i++;

		if (r == BUFSIZ)
		{
			(*buffer) = realloc((*buffer), BUFSIZ * (i + 1));
			ERROR_CHECK_NULL((*buffer), "Memory allocation error.");

			memset((*buffer) + read_total, 0, BUFSIZ);

			continue;
		}

		if (r < BUFSIZ)
		{
			break;
		}
	}

	if (r < 0)
	{
		error_log("Unable to read from socket. Errno %i.", errno);
		return -1;
	}
	
	return read_total;
}

void node_disconnect(Node node)
{
	assert(node);
	assert(node->sockfd);
	
	close(node->sockfd);
}

size_t node_sizeof(void)
{
	return sizeof(struct Node);
}
