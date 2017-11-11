#include <stddef.h>
#include "message.h"
#include "mem.h"
#include "assert.h"
#include "messages/version.h"

struct Message {
	int magic;
	int command;
	void *data;
};

Message message_new(int magic, int command) {
	Message r;
	
	assert(magic);
	assert(command);
	
	NEW(r);
	
	r->magic = magic;
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
