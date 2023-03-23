/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "mods/output.h"
#include "mods/error.h"

output_item output_new(void *content, size_t length)
{
    output_item item;

    assert(content);

    item = malloc(sizeof *item);
    if (item)
    {
        item->content = content;
        item->length = length;
        item->next = NULL;
    }

    return item;
}

output_item output_append(output_item x, output_item y)
{
    output_item head;

    assert(y);

    if (x == NULL)
    {
        return y;
    }

    head = x;

    while(x->next != NULL)
    {
        x = x->next;
    }

    x->next = y;

    return head;
}

output_item output_append_new(output_item head, void *content, size_t length)
{
    output_item item;

    item = output_new(content, length);

    if (item == NULL)
    {
        return NULL;
    }

    head = output_append(head, item);

    return head;
}

output_item output_append_new_copy(output_item head, void *content, size_t length)
{
    void *tmp;

    tmp = malloc(length);
    if (tmp == NULL)
    {
        return NULL;
    }

    memcpy(tmp, content, length);

    head = output_append_new(head, tmp, length);

    return head;
}

size_t output_size(output_item list)
{
    size_t size = 0;

    assert(list);

    while (list != NULL)
    {
        size += list->length;
        list = list->next;
    }

    return size;
}

void output_free(output_item list)
{
    output_item tmp;

    if (list == NULL)
    {
        return;
    }

    while (list)
    {
        tmp = list->next;
        free(list->content);
        free(list);
        list = tmp;
    }
}