#ifndef MESSAGE_H
#define MESSAGE_H 1

#define MESSAGE_COMMAND_VERSION 0

typedef struct Message *Message;

Message message_new(unsigned int);
size_t  message_serialize(Message, unsigned char **);
Message message_deserialize(unsigned char *, int);

#endif
