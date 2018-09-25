#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "crypto.h"
#include "message.h"
#include "serialize.h"
#include "mem.h"
#include "assert.h"

#define MESSAGE_MAINNET        0xD9B4BEF9
#define MESSAGE_TESTNET        0x0709110B
#define MESSAGE_COMMAND_MAXLEN 12
#define MESSAGE_PAYLOAD_MAXLEN 1024

struct Message {
	uint32_t       magic;
	char           command[MESSAGE_COMMAND_MAXLEN];
	uint32_t       length;
	uint32_t       checksum;
	unsigned char  payload[MESSAGE_PAYLOAD_MAXLEN];
};

int message_new(Message m, const char *command, unsigned char *payload, size_t len)
{
	int r;
	
	assert(m);
	assert(command);
	if (len)
	{
		assert(payload);
	}

	m->magic = MESSAGE_MAINNET;
	memcpy(m->command, command, strlen(command));
	m->length = len;
	if (len)
	{
		memcpy(m->payload, payload, len);
	}
	r = crypto_get_checksum(&m->checksum, m->payload, (size_t)m->length);
	if (r < 0)
	{
		return -1;
	}

	return 1;
}

size_t message_serialize(Message m, unsigned char **s) {
	unsigned char *head, *ptr;
	
	ptr = head = ALLOC(sizeof(struct Message) + MESSAGE_PAYLOAD_MAXLEN);
	
	// Serializing Message
	ptr = serialize_uint32(ptr, m->magic, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_char(ptr, m->command, MESSAGE_COMMAND_MAXLEN);
	ptr = serialize_uint32(ptr, m->length, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint32(ptr, m->checksum, SERIALIZE_ENDIAN_BIG);
	if (m->length)
		ptr = serialize_uchar(ptr, m->payload, m->length);
	
	*s = head;
	
	return 12 + MESSAGE_COMMAND_MAXLEN + m->length;
}

size_t message_deserialize(unsigned char *src, Message *dest, size_t l) {

	assert(src);
	assert(l >= 12 + MESSAGE_COMMAND_MAXLEN);

	if (*dest == NULL)
		NEW0(*dest);
	
	// De-Serializing Message
	src = deserialize_uint32(&((*dest)->magic), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_char((*dest)->command, src, MESSAGE_COMMAND_MAXLEN);
	src = deserialize_uint32(&((*dest)->length), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint32(&((*dest)->checksum), src, SERIALIZE_ENDIAN_BIG);
	if ((*dest)->length) {
		assert(l >= 12 + MESSAGE_COMMAND_MAXLEN + (*dest)->length);
		src = deserialize_uchar((*dest)->payload, src, (*dest)->length);
	}
	
	return 12 + MESSAGE_COMMAND_MAXLEN + (*dest)->length;
}

void message_free(Message m) {
	FREE(m);
}

int message_cmp_command(Message m, char *command) {
	assert(strlen(command) <= MESSAGE_COMMAND_MAXLEN);
	return strncmp(m->command, command, MESSAGE_COMMAND_MAXLEN);
}

int message_validate(Message m) {
	int r;
	uint32_t checksum;

	if (m->length == 0)
	{
		// return negative value
	}

	r = crypto_get_checksum(&checksum, m->payload, (size_t)m->length);
	if (r < 0)
	{
		// return negative value
	}

	return (checksum == m->checksum);
}

uint32_t message_get_payload_len(Message m) {
	assert(m);

	return m->length;
}

size_t message_get_payload(unsigned char **dest, Message m) {
	assert(m);

	if (*dest == NULL)
	{
		*dest = ALLOC(m->length);
	}

	memcpy(*dest, m->payload, m->length);
	
	return m->length;
}

size_t message_sizeof(void)
{
	return sizeof(struct Message);
}
