#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <stdint.h>

#define MESSAGE_COMMAND_VERSION 0
#define MESSAGE_COMMAND_VERACK  1

typedef struct Message *Message;

Message message_new(uint8_t);
size_t  message_serialize(Message, unsigned char **);
Message message_deserialize(unsigned char *, int);
void message_free(Message);

#endif
