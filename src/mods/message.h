/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <stdint.h>

#define MESSAGE_MIN_SIZE       24
#define MESSAGE_PAYLOAD_MAXLEN 1024
#define MESSAGE_COMMAND_MAXLEN 12

typedef struct Message *Message;
struct Message
{
    uint32_t       magic;
    char           command[MESSAGE_COMMAND_MAXLEN];
    uint32_t       length;
    uint32_t       checksum;
    unsigned char  payload[MESSAGE_PAYLOAD_MAXLEN];
};

int message_new(Message, const char *, unsigned char *, size_t);
int message_serialize(unsigned char *, size_t *, Message);
int message_deserialize(Message, unsigned char *, size_t);
int message_is_valid(Message);
int message_get_payload(unsigned char *output, Message m);
uint32_t message_get_payload_len(Message m);
size_t message_sizeof(void);
int message_is_complete(unsigned char *, size_t);

#endif
