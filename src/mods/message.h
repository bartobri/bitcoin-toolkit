/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <stdint.h>

#define MESSAGE_MIN_SIZE        24
#define MESSAGE_COMMAND_MAXLEN  12

#define MESSAGE_COMMAND_VERSION 1

typedef struct Message *Message;
struct Message
{
    uint32_t       magic;
    char           command[MESSAGE_COMMAND_MAXLEN];
    uint32_t       length;
    uint32_t       checksum;
    unsigned char  *payload;
};

int message_new(Message *, const char *, unsigned char *, size_t);
int message_to_raw(unsigned char *output, Message message);
int message_from_raw(Message, unsigned char *);
int message_is_valid(Message);
int message_is_complete(unsigned char *, size_t);
void message_destroy(Message);

#endif
