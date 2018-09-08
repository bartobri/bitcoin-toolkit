#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <inttypes.h>
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
// TODO - This version number should come from another source
#define USER_AGENT  "/Bitcoin-Toolkit:0.1.0/"

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

// Function Prototypes
static char *version_service_bit_to_str(int bit);
static int version_services_to_json(char *ptr, uint64_t value);

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
	r->user_agent_bytes = (uint64_t)strlen(USER_AGENT);
	
	if (r->user_agent_bytes) {
		r->user_agent = ALLOC(r->user_agent_bytes);
		memcpy(r->user_agent, USER_AGENT, strlen(USER_AGENT));
	} else {
		r->user_agent = NULL;
	}

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
	ptr = serialize_compuint(ptr, v->user_agent_bytes, SERIALIZE_ENDIAN_LIT);	
	ptr = serialize_char(ptr, v->user_agent, strlen(v->user_agent));
	ptr = serialize_uint32(ptr, v->start_height, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint8(ptr, v->relay, SERIALIZE_ENDIAN_LIT);
	
	*s = head;
	
	while (head++ != ptr)
		len++;

	return len;
}

size_t version_new_serialize(unsigned char **s) {
	size_t r;
	Version v;
	
	v = version_new();
	
	r = version_serialize(v, s);
	
	version_free(v);
	
	return r;
}

size_t version_deserialize(unsigned char *src, Version *dest, size_t l) {
	
	assert(src);
	assert(l >= 54 + IP_ADDR_FIELD_LEN + IP_ADDR_FIELD_LEN);
	
	if (*dest == NULL)
		NEW0(*dest);
	
	src = deserialize_uint32(&((*dest)->version), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint64(&((*dest)->services), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint64(&((*dest)->timestamp), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint64(&((*dest)->addr_recv_services), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uchar((*dest)->addr_recv_ip_address, src, IP_ADDR_FIELD_LEN);
	src = deserialize_uint16(&((*dest)->addr_recv_port), src, SERIALIZE_ENDIAN_BIG);
	src = deserialize_uint64(&((*dest)->addr_trans_services), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uchar((*dest)->addr_trans_ip_address, src, IP_ADDR_FIELD_LEN);
	src = deserialize_uint16(&((*dest)->addr_trans_port), src, SERIALIZE_ENDIAN_BIG);
	src = deserialize_uint64(&((*dest)->nonce), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_compuint(&((*dest)->user_agent_bytes), src, SERIALIZE_ENDIAN_LIT);
	if ((*dest)->user_agent_bytes) {
		assert(l >= 54 + IP_ADDR_FIELD_LEN + IP_ADDR_FIELD_LEN + (*dest)->user_agent_bytes);
		FREE((*dest)->user_agent);
		(*dest)->user_agent = ALLOC((*dest)->user_agent_bytes);
		src = deserialize_char((*dest)->user_agent, src, (*dest)->user_agent_bytes);
	} else {
		(*dest)->user_agent = NULL;
	}
	src = deserialize_uint32(&((*dest)->start_height), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint8(&((*dest)->relay), src, SERIALIZE_ENDIAN_LIT);
	
	return 61 + IP_ADDR_FIELD_LEN + IP_ADDR_FIELD_LEN + (*dest)->user_agent_bytes;
}

void version_free(Version v) {
	if (v && v->user_agent_bytes) {
		FREE(v->user_agent);
	}
	FREE(v);
}

char *version_to_json(Version v) {
	int i;
	char *head, *ptr;

	assert(v);

	head = ptr = ALLOC(1000);

	// Opening bracket
	ptr += sprintf(ptr, "{\n");

	// version
	ptr += sprintf(ptr, "  \"version\": %"PRIu32",\n", v->version);

	// services
	ptr += sprintf(ptr, "  \"services\": {\n");
	ptr += version_services_to_json(ptr, v->services);
	ptr += sprintf(ptr, "  },\n");

	// timestamp
	ptr += sprintf(ptr, "  \"timestamp\": %"PRIu64",\n", v->timestamp);

	// addr_recv_services
	ptr += sprintf(ptr, "  \"addr_recv_services\": {\n");
	ptr += version_services_to_json(ptr, v->addr_recv_services);
	ptr += sprintf(ptr, "  },\n");

	// addr_recv_ip_address
	ptr += sprintf(ptr, "  \"addr_recv_ip_address\": \"");
	for(i = 0; i < IP_ADDR_FIELD_LEN; ++i)
	{
		ptr += sprintf(ptr, "%02x", v->addr_recv_ip_address[i]);
	}
	ptr += sprintf(ptr, "\",\n");

	// addr_recv_port
	ptr += sprintf(ptr, "  \"addr_recv_port\": %"PRIu16",\n", v->addr_recv_port);

	// addr_trans_services
	ptr += sprintf(ptr, "  \"addr_trans_services\": {\n");
	ptr += version_services_to_json(ptr, v->addr_trans_services);
	ptr += sprintf(ptr, "  },\n");

	// addr_trans_ip_address
	ptr += sprintf(ptr, "  \"addr_trans_ip_address\": \"");
	for(i = 0; i < IP_ADDR_FIELD_LEN; ++i)
	{
		ptr += sprintf(ptr, "%02x", v->addr_trans_ip_address[i]);
	}
	ptr += sprintf(ptr, "\",\n");

	// addr_trans_port
	ptr += sprintf(ptr, "  \"addr_trans_port\": %"PRIu16",\n", v->addr_trans_port);

	// nonce
	ptr += sprintf(ptr, "  \"nonce\": %"PRIu64",\n", v->nonce);
	
	// user_agent
	ptr += sprintf(ptr, "  \"user_agent\": \"");
	for(i = 0; i < (int)v->user_agent_bytes; ++i)
	{
		ptr += sprintf(ptr, "%c", v->user_agent[i]);
	}
	ptr += sprintf(ptr, "\",\n");
	
	// start_height
	ptr += sprintf(ptr, "  \"start_height\": %"PRIu32",\n", v->start_height);
	
	// relay
	ptr += sprintf(ptr, "  \"relay\": %s\n", (v->relay == 0) ? "false" : "true");

	// Closing bracket
	sprintf(ptr, "}");

	return head;
}

static int version_services_to_json(char *ptr, uint64_t value) {
	int i, c, total;
	
	assert(ptr);

	c = total = 0;

	for (i = 0; i < 64; ++i)
	{
		if (((value >> i) & 0x0000000000000001) == 1)
		{
			c = sprintf(ptr, "    \"bit %d\": ", i + 1);
			total += c;
			ptr += c;
			c = sprintf(ptr, "\"%s\",\n", version_service_bit_to_str(i));
			total += c;
			ptr += c;
		}
	}

	// removing the trailing comma if we printed a list
	if (total > 0)
	{
		ptr--;
		ptr--;
		sprintf(ptr, "\n");
		total--;
	}

	return total;
}

static char *version_service_bit_to_str(int bit) {
	assert(bit >= 0 && bit < 64);

	switch (bit)
	{
		case 0:
			return "NODE_NETWORK";
			break;
		case 1:
			return "NODE_GETUTXO";
			break;
		case 2:
			return "NODE_BLOOM";
			break;
		case 3:
			return "NODE_WITNESS";
			break;
		case 10:
			return "NODE_NETWORK_LIMITED";
			break;
	}

	return "UNKNOWN";
}