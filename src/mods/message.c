#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "crypto.h"
#include "message.h"
#include "serialize.h"
#include "mem.h"
#include "assert.h"
#include "commands/version.h"

#define MESSAGE_MAINNET        0xD9B4BEF9
#define MESSAGE_TESTNET        0x0709110B
#define MESSAGE_COMMAND_MAXLEN 12
#define MESSAGE_PAYLOAD_MAXLEN 1024

const char *message_commands[] = {
	[MESSAGE_COMMAND_VERSION] = "version",
	[MESSAGE_COMMAND_VERACK]  = "verack",
};

struct Message {
	uint8_t        type;
	uint32_t       magic;
	unsigned char  command[MESSAGE_COMMAND_MAXLEN];
	uint32_t       length;
	uint32_t       checksum;
	unsigned char *payload;
};

Message message_new(uint8_t type) {
	Message m;
	
	assert(type <= MESSAGE_COMMAND_VERACK);
	
	NEW0(m);

	m->type = type;
	m->magic = MESSAGE_MAINNET;
	memcpy(m->command, message_commands[m->type], strlen(message_commands[m->type]));
	
	switch(type) {
		case MESSAGE_COMMAND_VERSION:
			m->length = (uint32_t)version_new_serialize(&(m->payload));
			break;
		case MESSAGE_COMMAND_VERACK:
			m->length = 0;
			break;
		default:
			assert(0);
			break;
	}
	
	m->checksum = crypto_get_checksum(m->payload, (size_t)m->length);

	return m;
}

size_t message_serialize(Message m, unsigned char **s) {
	unsigned char *head, *ptr;
	
	ptr = head = ALLOC(sizeof(struct Message) + MESSAGE_PAYLOAD_MAXLEN);
	
	// Serializing Message
	ptr = serialize_uint32(ptr, m->magic, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uchar(ptr, m->command, MESSAGE_COMMAND_MAXLEN);
	ptr = serialize_uint32(ptr, m->length, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint32(ptr, m->checksum, SERIALIZE_ENDIAN_BIG);
	if (m->length)
		ptr = serialize_uchar(ptr, m->payload, m->length);
	
	*s = head;
	
	return 12 + MESSAGE_COMMAND_MAXLEN + m->length;
}

Message message_deserialize(unsigned char *data, int l) {
	Message m;

	assert(data);
	assert(l);

	NEW0(m);
	
	// De-Serializing Message
	data = deserialize_uint32(&(m->magic), data, SERIALIZE_ENDIAN_LIT);
	data = deserialize_uchar(m->command, data, MESSAGE_COMMAND_MAXLEN);
	data = deserialize_uint32(&(m->length), data, SERIALIZE_ENDIAN_LIT);
	data = deserialize_uint32(&(m->checksum), data, SERIALIZE_ENDIAN_BIG);
	m->payload = ALLOC(m->length);
	data = deserialize_uchar(m->payload, data, m->length);
	
	return m;
}
