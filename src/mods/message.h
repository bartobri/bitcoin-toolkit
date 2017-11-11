#ifndef MESSAGE_H
#define MESSAGE_H 1

#define MESSAGE_MAGIC_MAINNET      1
#define MESSAGE_COMMAND_VERSION    1

typedef struct Message *Message;

Message message_new(int, int);

#endif
