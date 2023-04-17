/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stddef.h>
#include "stub.h"


leveldb_options_t* leveldb_options_create()
{
	return NULL;
}

void leveldb_options_set_compression(leveldb_options_t *stub, int i)
{
	(void)i;
	(void)stub;
}

void leveldb_options_set_create_if_missing(leveldb_options_t *stub, unsigned char c)
{
	(void)c;
	(void)stub;
}

void leveldb_options_set_error_if_exists(leveldb_options_t *stub, unsigned char c)
{
	(void)c;
	(void)stub;
}

void leveldb_options_set_filter_policy(leveldb_options_t *stub, leveldb_filterpolicy_t *filter)
{
	(void)stub;
	(void)filter;
}

leveldb_t* leveldb_open(const leveldb_options_t* options, const char* name, char** errptr)
{
	(void)options;
	(void)name;
	(void)errptr;

	return NULL;
}

leveldb_readoptions_t* leveldb_readoptions_create()
{
	return NULL;
}

leveldb_iterator_t* leveldb_create_iterator(leveldb_t* db, const leveldb_readoptions_t* options)
{
	(void)db;
	(void)options;

	return NULL;
}

leveldb_filterpolicy_t* leveldb_filterpolicy_create_bloom(int bits_per_key)
{
	(void)bits_per_key;

	return NULL;
}

void leveldb_iter_seek_to_first(leveldb_iterator_t* stub)
{
	(void)stub;
}

void leveldb_iter_seek(leveldb_iterator_t* stub, const char* k, size_t klen)
{
	(void)stub;
	(void)k;
	(void)klen;
}

unsigned char leveldb_iter_valid(const leveldb_iterator_t* stub)
{
	(void)stub;

	return 0;
}

void leveldb_iter_next(leveldb_iterator_t* stub)
{
	(void)stub;
}

const char* leveldb_iter_key(const leveldb_iterator_t* stub, size_t* klen)
{
	(void)stub;
	(void)klen;

	return NULL;
}

const char* leveldb_iter_value(const leveldb_iterator_t* stub, size_t* vlen)
{
	(void)stub;
	(void)vlen;

	return NULL;
}

char* leveldb_get(leveldb_t* db, const leveldb_readoptions_t* options, const char* key, size_t keylen, size_t* vallen, char** errptr)
{
	(void)db;
	(void)options;
	(void)key;
	(void)keylen;
	(void)vallen;
	(void)errptr;

	return NULL;
}

void leveldb_delete(leveldb_t* db, const leveldb_writeoptions_t* options, const char* key, size_t keylen, char** errptr)
{
	(void)db;
	(void)options;
	(void)key;
	(void)keylen;
	(void)errptr;
}

leveldb_writeoptions_t* leveldb_writeoptions_create()
{
	return NULL;
}

void leveldb_put(leveldb_t* db, const leveldb_writeoptions_t* options, const char* key, size_t keylen, const char* val, size_t vallen, char** errptr)
{
	(void)db;
	(void)options;
	(void)key;
	(void)keylen;
	(void)val;
	(void)vallen;
	(void)errptr;
}

void leveldb_write(leveldb_t* db, const leveldb_writeoptions_t* options, leveldb_writebatch_t* batch, char** errptr)
{
	(void)db;
	(void)options;
	(void)batch;
	(void)errptr;
}

leveldb_writebatch_t* leveldb_writebatch_create(void)
{
	return NULL;
}

void leveldb_writebatch_put(leveldb_writebatch_t *batch, const char* key, size_t klen, const char* val, size_t vlen)
{
	(void)batch;
	(void)key;
	(void)klen;
	(void)val;
	(void)vlen;
}

void leveldb_writebatch_clear(leveldb_writebatch_t *batch)
{
	(void)batch;
}

void leveldb_writebatch_delete(leveldb_writebatch_t *batch, const char* key, size_t klen)
{
	(void)batch;
	(void)key;
	(void)klen;
}

void leveldb_writebatch_destroy(leveldb_writebatch_t *batch)
{
	(void)batch;
}

void leveldb_writeoptions_destroy(leveldb_writeoptions_t* stub)
{
	(void)stub;

}

void leveldb_readoptions_destroy(leveldb_readoptions_t* stub)
{
	(void)stub;
}

void leveldb_iter_destroy(leveldb_iterator_t* stub)
{
	(void)stub;
}

void leveldb_close(leveldb_t* db)
{
	(void)db;
}