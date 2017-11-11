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

struct Message {
	uint32_t magic;
	char     command[MESSAGE_COMMAND_LEN];
	uint32_t length;
	uint32_t checksum;
	unsigned char *payload;
};

Message message_new(const char *c) {
	Message r;
	
	assert(c);
	
	NEW0(r);
	
	r->magic = MESSAGE_MAINNET;
	strncpy(r->command, c, MESSAGE_COMMAND_MAXLEN);
	
	if (strcmp(r->command, "version")) {
		r->payload = version_serialize(version_new());
	} else {
		r->payload = NULL;
	}
	
	return r;
}
