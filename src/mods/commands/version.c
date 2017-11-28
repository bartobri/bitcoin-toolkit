#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "version.h"
#include "mods/serialize.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define IP_ADDR_FIELD_LEN  16
#define USER_AGENT_MAX_LEN 1024

#define VERSION     70015
#define SERVICES    0x00
#define IP_ADDRESS  "00000000000000000000ffff7f000001"
#define PORT        8333
#define USER_AGENT  "/Test User Agent/"

struct Version {
	uint32_t  version;
	uint64_t services;
	uint64_t  timestamp;
	uint64_t addr_recv_services;
	unsigned char addr_recv_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_recv_port;
	uint64_t addr_trans_services;
	unsigned char addr_trans_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_trans_port;
	uint64_t nonce;
	uint64_t user_agent_bytes;
	char*    user_agent;
	uint32_t  start_height;
	uint8_t   relay;
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
	
	r->addr_trans_port = PORT;
	r->nonce = 0x00;
	r->user_agent_bytes = ((uint64_t)strlen(USER_AGENT)) << 56;
	r->user_agent = USER_AGENT;
	r->start_height = 0x00;
	r->relay = 0x00;
	
	return r;
}

size_t version_serialize(Version v, unsigned char **s) {
	size_t len = 0;
	unsigned char *head, *ptr;
	
	ptr = head = ALLOC(sizeof(struct Version) + USER_AGENT_MAX_LEN);
	
	ptr = serialize_uint32(ptr, v->version, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint64(ptr, v->services, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint64(ptr, v->timestamp, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint64(ptr, v->addr_recv_services, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uchar(ptr, v->addr_recv_ip_address, IP_ADDR_FIELD_LEN);
	ptr = serialize_uint16(ptr, v->addr_recv_port, SERIALIZE_ENDIAN_BIG);
	ptr = serialize_uint64(ptr, v->addr_trans_services, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uchar(ptr, v->addr_trans_ip_address, IP_ADDR_FIELD_LEN);
	ptr = serialize_uint16(ptr, v->addr_trans_port, SERIALIZE_ENDIAN_BIG);
	ptr = serialize_uint64(ptr, v->nonce, SERIALIZE_ENDIAN_LIT);
	
	// TODO - handle variable length ints properly
	*ptr++ = (unsigned char)((v->user_agent_bytes & 0xFF00000000000000) >> 56);
	
	ptr = serialize_char(ptr, v->user_agent, strlen(v->user_agent));
	ptr = serialize_uint32(ptr, v->start_height, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint8(ptr, v->relay, SERIALIZE_ENDIAN_LIT);
	
	*s = head;
	
	while (head++ != ptr)
		len++;

	return len;
}
