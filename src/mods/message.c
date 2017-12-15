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
	unsigned char *payload;
};

Message message_new(const char *command, unsigned char *payload, size_t len) {
	Message m;
	
	assert(command);
	
	NEW0(m);

	m->magic = MESSAGE_MAINNET;
	memcpy(m->command, command, strlen(command));
	m->length = len;
	if (len) {
		m->payload = ALLOC(len);
		memcpy(m->payload, payload, len);
	}
	m->checksum = crypto_get_checksum(m->payload, (size_t)m->length);

	return m;
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
		FREE((*dest)->payload);
		(*dest)->payload = ALLOC((*dest)->length);
		src = deserialize_uchar((*dest)->payload, src, (*dest)->length);
	} else {
		(*dest)->payload = NULL;
	}
	
	return 12 + MESSAGE_COMMAND_MAXLEN + (*dest)->length;
}

void message_free(Message m) {
	if (m) {
		FREE(m->payload);
	}
	FREE(m);
}

int message_cmp_command(Message m, char *command) {
	assert(strlen(command) <= MESSAGE_COMMAND_MAXLEN);
	return strncmp(m->command, command, MESSAGE_COMMAND_MAXLEN);
}
