/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef VERSION_H
#define VERSION_H 1

#include <stddef.h>

#define IP_ADDR_FIELD_LEN  16
#define USER_AGENT_MAX_LEN 1024

#define VERSION_COMMAND "version"

typedef struct Version *Version;
struct Version
{
	uint32_t version;
	uint64_t services;
	uint64_t timestamp;
	uint64_t addr_recv_services;
	unsigned char addr_recv_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_recv_port;
	uint64_t addr_trans_services;
	unsigned char addr_trans_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_trans_port;
	uint64_t nonce;
	uint64_t user_agent_bytes;
	char    *user_agent;
	uint32_t start_height;
	uint8_t  relay;
};

int version_new(Version *);
int version_to_raw(unsigned char *, Version);
int version_new_from_raw(Version *, unsigned char *, size_t);
int version_to_json(char **, Version);
char *version_service_bit_to_str(int);
void version_destroy(Version);

#endif
