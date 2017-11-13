#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "message.h"
#include "mem.h"
#include "assert.h"
#include "messages/version.h"

#define MESSAGE_COMMAND_MAXLEN 12
#define MESSAGE_MAINNET        0xD9B4BEF9
#define MESSAGE_TESTNET        0x0709110B
#define MESSAGE_MAX_PAYLOAD    1024

struct Message {
	uint32_t       magic;
	char           command[MESSAGE_COMMAND_MAXLEN];
	uint32_t       length;
	uint32_t       checksum;
	unsigned char *payload;
};

Message message_new(const char *c) {
	Message m;

	assert(c);

	NEW0(m);

	m->magic = MESSAGE_MAINNET;
	strncpy(m->command, c, MESSAGE_COMMAND_MAXLEN);

	if (strcmp(m->command, "version") == 0) {
		m->length = (uint32_t)version_serialize(version_new(), &(m->payload));
	} else {
		m->payload = NULL;
	}

	return m;
}

size_t message_serialize(Message m, unsigned char **s) {
	//int i;
	size_t len = 0;
	unsigned char *temp;
	
	temp = ALLOC(sizeof(struct Message) + MESSAGE_MAX_PAYLOAD);
	
	// Serializing Magic
	temp[0] = (unsigned char)((m->magic & 0xFF000000) >> 24);
	temp[1] = (unsigned char)((m->magic & 0x00FF0000) >> 16);
	temp[2] = (unsigned char)((m->magic & 0x0000FF00) >> 8);
	temp[3] = (unsigned char)(m->magic & 0x000000FF);
	len += 4;
	
	*s = temp;
	
	return len;
}
