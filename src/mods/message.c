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
};

struct Message {
	uint32_t       magic;
	unsigned char  command[MESSAGE_COMMAND_MAXLEN];
	uint32_t       length;
	uint32_t       checksum;
	unsigned char *payload;
};

Message message_new(unsigned int type) {
	Message m;
	
	NEW0(m);

	m->magic = MESSAGE_MAINNET;
	
	switch(type) {
		case MESSAGE_COMMAND_VERSION:
			memcpy(m->command, message_commands[MESSAGE_COMMAND_VERSION], strlen(message_commands[MESSAGE_COMMAND_VERSION]));
			Version version = version_new();
			m->length = (uint32_t)version_serialize(version, &(m->payload));
			FREE(version);
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
