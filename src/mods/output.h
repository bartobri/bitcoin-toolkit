/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef OUTPUT_H
#define OUTPUT_H 1

typedef struct output_item *output_list;
struct output_item {
    void *content;
    size_t length;
    output_list next;
};

output_list output_new(void *, size_t);
output_list output_append(output_list, output_list);
output_list output_append_new(output_list, void *, size_t);
output_list output_append_new_copy(output_list, void *, size_t);
void output_free(output_list);

#endif