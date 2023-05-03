/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef DATABASE_H
#define DATABASE_H 1

#include <stdbool.h>

typedef struct DBRef *DBRef;

int database_open(DBRef *, char *, bool);
int database_is_open(DBRef);
int database_iter_seek_start(DBRef);
int database_iter_seek_key(DBRef, unsigned char *, size_t);
int database_iter_next(DBRef);
int database_iter_get(unsigned char **, size_t *, unsigned char **, size_t *, DBRef);
int database_iter_get_value(unsigned char **, size_t *, DBRef);
int database_iter_reset(DBRef);
int database_get(unsigned char **, size_t *, DBRef, unsigned char *, size_t);
int database_put(DBRef, unsigned char *, size_t, unsigned char *, size_t);
int database_delete(DBRef, unsigned char *, size_t);
void database_close(DBRef);

int database_batch_put(DBRef, unsigned char *, size_t, unsigned char *, size_t);
int database_batch_delete(DBRef, unsigned char *, size_t);
int database_batch_write(DBRef);

#endif
