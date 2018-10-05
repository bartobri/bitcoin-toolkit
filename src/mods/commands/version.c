#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <inttypes.h>
#include "version.h"
#include "mods/config.h"
#include "mods/serialize.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x)  STRINGIFY2(x)

#define IP_ADDR_FIELD_LEN  16
#define USER_AGENT_MAX_LEN 1024

#define VERSION     70015
#define SERVICES    0x00
#define IP_ADDRESS  "00000000000000000000ffff7f000001"
#define PORT        8333
#define USER_AGENT  "/Bitcoin-Toolkit:" STRINGIFY(BTK_VERSION_MAJOR) "." STRINGIFY(BTK_VERSION_MINOR) "." STRINGIFY(BTK_VERSION_REVISION) "/"

struct Version {
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
	char     user_agent[USER_AGENT_MAX_LEN];
	uint32_t start_height;
	uint8_t  relay;
};

// Function Prototypes
static char *version_service_bit_to_str(int bit);
static int version_services_to_json(char *ptr, uint64_t value);

int version_new(Version v)
{
	int r;
	
	v->version = VERSION;
	v->services = SERVICES;
	v->timestamp = time(NULL);
	v->addr_recv_services = SERVICES;

	if (strlen(IP_ADDRESS) / 2 > IP_ADDR_FIELD_LEN)
	{
		return -1;
	}

	r = hex_str_to_raw(v->addr_recv_ip_address, IP_ADDRESS);
	if (r < 0)
	{
		return -1;
	}
	
	v->addr_recv_port = PORT;
	v->addr_trans_services = SERVICES;
	
	r = hex_str_to_raw(v->addr_trans_ip_address, IP_ADDRESS);
	if (r < 0)
	{
		return -1;
	}
	
	v->addr_trans_port = PORT;
	v->nonce = 0x00;
	v->user_agent_bytes = (uint64_t)strlen(USER_AGENT);
	
	if (v->user_agent_bytes)
	{
		memcpy(v->user_agent, USER_AGENT, strlen(USER_AGENT));
	}

	v->start_height = 0x00;
	v->relay = 0x00;
	
	return 1;
}

int version_serialize(unsigned char *output, Version v)
{
	int r;
	unsigned char *head;

	r = 0;
	head = output;
	
	output = serialize_uint32(output, v->version, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, v->services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, v->timestamp, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint64(output, v->addr_recv_services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uchar(output, v->addr_recv_ip_address, IP_ADDR_FIELD_LEN);
	output = serialize_uint16(output, v->addr_recv_port, SERIALIZE_ENDIAN_BIG);
	output = serialize_uint64(output, v->addr_trans_services, SERIALIZE_ENDIAN_LIT);
	output = serialize_uchar(output, v->addr_trans_ip_address, IP_ADDR_FIELD_LEN);
	output = serialize_uint16(output, v->addr_trans_port, SERIALIZE_ENDIAN_BIG);
	output = serialize_uint64(output, v->nonce, SERIALIZE_ENDIAN_LIT);
	output = serialize_compuint(output, v->user_agent_bytes, SERIALIZE_ENDIAN_LIT);	
	output = serialize_char(output, v->user_agent, strlen(v->user_agent));
	output = serialize_uint32(output, v->start_height, SERIALIZE_ENDIAN_LIT);
	output = serialize_uint8(output, v->relay, SERIALIZE_ENDIAN_LIT);
	
	while (head++ != output)
		r++;

	return r;
}

int version_new_serialize(unsigned char *output)
{
	int r;
	Version v;

	NEW(v);

	r = version_new(v);
	if (r < 0)
	{
		return -1;
	}
	
	r = version_serialize(output, v);
	if (r < 0)
	{
		return -1;
	}
	
	FREE(v);
	
	return r;
}

int version_deserialize(Version output, unsigned char *input, size_t input_len)
{
	assert(output);
	assert(input);
	assert(input_len);

	if (input_len < 85)
	{
		return -1;
	}
	
	input = deserialize_uint32(&(output->version), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&(output->services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&(output->timestamp), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint64(&(output->addr_recv_services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar(output->addr_recv_ip_address, input, IP_ADDR_FIELD_LEN);
	input = deserialize_uint16(&(output->addr_recv_port), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint64(&(output->addr_trans_services), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uchar(output->addr_trans_ip_address, input, IP_ADDR_FIELD_LEN);
	input = deserialize_uint16(&(output->addr_trans_port), input, SERIALIZE_ENDIAN_BIG);
	input = deserialize_uint64(&(output->nonce), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_compuint(&(output->user_agent_bytes), input, SERIALIZE_ENDIAN_LIT);
	if (output->user_agent_bytes)
	{
		if (input_len < 85 + output->user_agent_bytes)
		{
			return -1;
		}
		input = deserialize_char(output->user_agent, input, output->user_agent_bytes);
	}
	input = deserialize_uint32(&(output->start_height), input, SERIALIZE_ENDIAN_LIT);
	input = deserialize_uint8(&(output->relay), input, SERIALIZE_ENDIAN_LIT);
	
	return 1;
}

int version_to_json(char *output, Version v)
{
	int i;

	assert(output);
	assert(v);

	output += sprintf(output, "{\n");
	output += sprintf(output, "  \"version\": %"PRIu32",\n", v->version);
	output += sprintf(output, "  \"services\": {\n");
	output += version_services_to_json(output, v->services);
	output += sprintf(output, "  },\n");
	output += sprintf(output, "  \"timestamp\": %"PRIu64",\n", v->timestamp);
	output += sprintf(output, "  \"addr_recv_services\": {\n");
	output += version_services_to_json(output, v->addr_recv_services);
	output += sprintf(output, "  },\n");
	output += sprintf(output, "  \"addr_recv_ip_address\": \"");
	for(i = 0; i < IP_ADDR_FIELD_LEN; ++i)
	{
		output += sprintf(output, "%02x", v->addr_recv_ip_address[i]);
	}
	output += sprintf(output, "\",\n");
	output += sprintf(output, "  \"addr_recv_port\": %"PRIu16",\n", v->addr_recv_port);
	output += sprintf(output, "  \"addr_trans_services\": {\n");
	output += version_services_to_json(output, v->addr_trans_services);
	output += sprintf(output, "  },\n");
	output += sprintf(output, "  \"addr_trans_ip_address\": \"");
	for(i = 0; i < IP_ADDR_FIELD_LEN; ++i)
	{
		output += sprintf(output, "%02x", v->addr_trans_ip_address[i]);
	}
	output += sprintf(output, "\",\n");
	output += sprintf(output, "  \"addr_trans_port\": %"PRIu16",\n", v->addr_trans_port);
	output += sprintf(output, "  \"nonce\": %"PRIu64",\n", v->nonce);
	output += sprintf(output, "  \"user_agent\": \"");
	for(i = 0; i < (int)v->user_agent_bytes; ++i)
	{
		output += sprintf(output, "%c", v->user_agent[i]);
	}
	output += sprintf(output, "\",\n");
	output += sprintf(output, "  \"start_height\": %"PRIu32",\n", v->start_height);
	output += sprintf(output, "  \"relay\": %s\n", (v->relay == 0) ? "false" : "true");
	sprintf(output, "}");

	return 1;
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

size_t version_sizeof(void)
{
	return sizeof(struct Version);
}