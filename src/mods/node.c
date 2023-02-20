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

int node_connect(Node node, const char *host, const char *service)
{
	int r, sockfd;
	struct addrinfo *hints;
	struct addrinfo *result;
	
	assert(node);
	assert(host);
	assert(service);

	hints = malloc(sizeof(*hints));
	ERROR_CHECK_NULL(hints, "Memory allocation error.");

	memset(hints, 0, sizeof(*hints));

	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_STREAM;

	r = getaddrinfo(host, service, hints, &result);
	if (r > 0)
	{
		error_log("Can not get address info. Error code %i.", r);
		return -1;
	}

	// Set up a socket
	sockfd = socket(result->ai_family, result->ai_socktype, 0);
	if (sockfd < 0)
	{
		error_log("Unable to create new socket. Errno %i.", errno);
		return -1;
	}

	// Connect to server
	r = connect(sockfd, result->ai_addr, result->ai_addrlen);
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
	int r;
	int read_total = 0;
	int buf_size = BUFSIZ;
	
	assert(node);
	assert(buffer);

	*buffer = malloc(buf_size);
	ERROR_CHECK_NULL((*buffer), "Memory allocation error.");

	while ((r = read(node->sockfd, (*buffer) + read_total, buf_size - read_total)) > 0)
	{
		read_total += r;

		if (read_total == buf_size)
		{
			buf_size = buf_size * 2;

			(*buffer) = realloc((*buffer), buf_size);
			ERROR_CHECK_NULL((*buffer), "Memory allocation error.");

			continue;
		}
	}

	if (r < 0)
	{
		error_log("Unable to read from socket. Errno %i.", errno);
		return -1;
	}

	memset((*buffer) + read_total, 0, buf_size - read_total);
	
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
