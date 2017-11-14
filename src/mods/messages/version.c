#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "version.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define VERSION 70015

struct Version {
	int32_t  version;
	uint64_t services;
	int64_t  timestamp;
	uint64_t addr_recv_services;
	char     addr_recv_ip_address;
	uint16_t addr_recv_port;
	uint64_t addr_trans_services;
	char     addr_trans_ip_address;
	uint16_t addr_trans_port;
	uint64_t nonce;
	uint64_t user_agent_bytes;
	char*    user_agent;
	int32_t  start_height;
	int8_t   relay;
};

Version version_new(void) {
	Version r;
	
	NEW(r);
	
	r->version = VERSION;
	
	return r;
}

// TODO - need to also implement a length argument
size_t version_serialize(Version v, unsigned char **s) {
	size_t len = 0;
	unsigned char *temp;
	
	temp = ALLOC(sizeof(struct Version));
	
	// Serializing Length (little endian)
	temp[len++] = (unsigned char)(v->version & 0x000000FF);
	temp[len++] = (unsigned char)((v->version & 0x0000FF00) >> 8);
	temp[len++] = (unsigned char)((v->version & 0x00FF0000) >> 16);
	temp[len++] = (unsigned char)((v->version & 0xFF000000) >> 24);
	
	*s = temp;

	return len;
}
