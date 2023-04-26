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
#include "message.h"

#define MAX_MESSAGE_QUEUE 100

struct Node
{
	int sockfd;
	char *host;
	char *service;
};

int node_new(Node *node, char *host, char *service)
{
	assert(host);
	assert(service);

	*node = malloc(sizeof(struct Node));
	ERROR_CHECK_NULL(*node, "Memory allocation error.");

	(*node)->host = host;
	(*node)->service = service;

	return 1;
}

int node_connect(Node node)
{
	int r, sockfd;
	struct addrinfo *hints;
	struct addrinfo *result;
	
	assert(node);

	hints = malloc(sizeof(*hints));
	ERROR_CHECK_NULL(hints, "Memory allocation error.");

	memset(hints, 0, sizeof(*hints));

	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_STREAM;

	r = getaddrinfo(node->host, node->service, hints, &result);
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
		error_log("Unable to connect to host %s. Errno %i.", node->host, errno);
		return -1;
	}

	// If we get here, connection succeeded. Set up node struct.
	node->sockfd = sockfd;

	free(hints);

	// Freeing result (linked list)
	struct addrinfo *tmp;
	do {
		tmp = result->ai_next;
		free(result);
		result = tmp;
	} while(result != NULL);
	
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

int node_read_message(unsigned char **message, Node node)
{
	int r;
	uint32_t read_total = 0;
	uint32_t message_len;
	uint32_t payload_len;

	assert(node);

	(*message) = malloc(MESSAGE_MIN_SIZE);
	ERROR_CHECK_NULL((*message), "Memory allocation error.");

	while ((r = read(node->sockfd, (*message) + read_total, MESSAGE_MIN_SIZE - read_total)) > 0)
	{
		read_total += r;

		if (read_total == MESSAGE_MIN_SIZE)
		{
			break;
		}
	}
	if (r < 0)
	{
		error_log("Unable to read from socket. Errno %i.", errno);
		return -1;
	}

	r = message_get_payload_len(&payload_len, *message);
	ERROR_CHECK_NEG(r, "Could not get payload length.");

	message_len = MESSAGE_MIN_SIZE + payload_len;

	(*message) = realloc((*message), message_len);
	ERROR_CHECK_NULL((*message), "Memory allocation error.");

	while ((r = read(node->sockfd, (*message) + read_total, message_len - read_total)) > 0)
	{
		read_total += r;

		if (read_total == message_len)
		{
			break;
		}
	}

	return (int)read_total;
}

void node_disconnect(Node node)
{
	assert(node);
	assert(node->sockfd);
	
	close(node->sockfd);
}

void node_destroy(Node node)
{
	assert(node);

	free(node);
}
