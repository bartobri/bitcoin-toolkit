#include <stddef.h>
#include <stdint.h>
#include "message.h"
#include "mem.h"
#include "assert.h"
#include "messages/version.h"

#define MESSAGE_MAINNET 0xD9B4BEF9
#define MESSAGE_TESTNET 0x0709110B

struct Message {
	uint32_t magic;
	int command;
	void *data;
};

Message message_new(int command) {
	Message r;
	
	assert(command);
	
	NEW(r);
	
	r->magic = MESSAGE_MAINNET;
	r->command = command;
	
	switch(command) {
		case MESSAGE_COMMAND_VERSION:
			r->data = version_new();
			break;
		default:
			r->data = NULL;
	}
	
	return r;
}
