#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <stdint.h>

typedef struct Message *Message;

int message_new(Message, const char *, unsigned char *, size_t);
int message_serialize(unsigned char *, size_t *, Message);
int message_deserialize(Message, unsigned char *, size_t);
int message_cmp_command(Message, char *);
int message_validate(Message);
int message_get_payload(unsigned char *output, Message m);
uint32_t message_get_payload_len(Message m);
size_t message_sizeof(void);

#endif
