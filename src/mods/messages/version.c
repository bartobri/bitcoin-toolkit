#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "version.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define IP_ADDR_FIELD_LEN 16

#define VERSION  70015
#define SERVICES 0x00
#define IP_ADDRESS "00000000000000000000ffff7f000001"
#define PORT 8333

struct Version {
	int32_t  version;
	uint64_t services;
	int64_t  timestamp;
	uint64_t addr_recv_services;
	unsigned char addr_recv_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_recv_port;
	uint64_t addr_trans_services;
	unsigned char addr_trans_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_trans_port;
	uint64_t nonce;
	uint64_t user_agent_bytes;
	char*    user_agent;
	int32_t  start_height;
	int8_t   relay;
};

Version version_new(void) {
	Version r;
	unsigned char *temp;
	
	NEW(r);
	
	r->version = VERSION;
	r->services = SERVICES;
	r->timestamp = time(NULL);
	r->addr_recv_services = SERVICES;

	temp = hex_str_to_uc(IP_ADDRESS);
	memcpy(r->addr_recv_ip_address, temp, IP_ADDR_FIELD_LEN);
	FREE(temp);
	
	r->addr_recv_port = PORT;
	r->addr_trans_services = SERVICES;
	
	temp = hex_str_to_uc(IP_ADDRESS);
	memcpy(r->addr_trans_ip_address, temp, IP_ADDR_FIELD_LEN);
	FREE(temp);
	
	return r;
}

size_t version_serialize(Version v, unsigned char **s) {
	size_t len = 0;
	unsigned char *temp;
	
	temp = ALLOC(sizeof(struct Version));
	
	// Serializing Version (little endian)
	temp[len++] = (unsigned char)(v->version & 0x000000FF);
	temp[len++] = (unsigned char)((v->version & 0x0000FF00) >> 8);
	temp[len++] = (unsigned char)((v->version & 0x00FF0000) >> 16);
	temp[len++] = (unsigned char)((v->version & 0xFF000000) >> 24);
	
	// Serializing Services (little endian)
	temp[len++] = (unsigned char)(v->services & 0x00000000000000FF);
	temp[len++] = (unsigned char)((v->services & 0x000000000000FF00) >> 8);
	temp[len++] = (unsigned char)((v->services & 0x0000000000FF0000) >> 16);
	temp[len++] = (unsigned char)((v->services & 0x00000000FF000000) >> 24);
	temp[len++] = (unsigned char)((v->services & 0x000000FF00000000) >> 32);
	temp[len++] = (unsigned char)((v->services & 0x0000FF0000000000) >> 40);
	temp[len++] = (unsigned char)((v->services & 0x00FF000000000000) >> 48);
	temp[len++] = (unsigned char)((v->services & 0xFF00000000000000) >> 56);
	
	// Serializing Timestamp (little endian)
	temp[len++] = (unsigned char)(v->timestamp & 0x00000000000000FF);
	temp[len++] = (unsigned char)((v->timestamp & 0x000000000000FF00) >> 8);
	temp[len++] = (unsigned char)((v->timestamp & 0x0000000000FF0000) >> 16);
	temp[len++] = (unsigned char)((v->timestamp & 0x00000000FF000000) >> 24);
	temp[len++] = (unsigned char)((v->timestamp & 0x000000FF00000000) >> 32);
	temp[len++] = (unsigned char)((v->timestamp & 0x0000FF0000000000) >> 40);
	temp[len++] = (unsigned char)((v->timestamp & 0x00FF000000000000) >> 48);
	temp[len++] = (unsigned char)((v->timestamp & 0xFF00000000000000) >> 56);
	
	// Serializing Addr Rec Services (little endian)
	temp[len++] = (unsigned char)(v->addr_recv_services & 0x00000000000000FF);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0x000000000000FF00) >> 8);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0x0000000000FF0000) >> 16);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0x00000000FF000000) >> 24);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0x000000FF00000000) >> 32);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0x0000FF0000000000) >> 40);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0x00FF000000000000) >> 48);
	temp[len++] = (unsigned char)((v->addr_recv_services & 0xFF00000000000000) >> 56);

	// Serializing Addr Rec IP Address (big endian)
	memcpy(temp + len, v->addr_recv_ip_address, IP_ADDR_FIELD_LEN);
	len += IP_ADDR_FIELD_LEN;
	
	// Serializing Port (big endian)
	temp[len++] = (unsigned char)((v->addr_recv_port & 0xFF00) >> 8);
	temp[len++] = (unsigned char)(v->addr_recv_port & 0x00FF);
	
	// Serializing Addr Trans Services (little endian)
	temp[len++] = (unsigned char)(v->addr_trans_services & 0x00000000000000FF);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0x000000000000FF00) >> 8);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0x0000000000FF0000) >> 16);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0x00000000FF000000) >> 24);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0x000000FF00000000) >> 32);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0x0000FF0000000000) >> 40);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0x00FF000000000000) >> 48);
	temp[len++] = (unsigned char)((v->addr_trans_services & 0xFF00000000000000) >> 56);
	
	// Serializing Addr Rec IP Address (big endian)
	memcpy(temp + len, v->addr_trans_ip_address, IP_ADDR_FIELD_LEN);
	len += IP_ADDR_FIELD_LEN;
	
	*s = temp;

	return len;
}
