/*
 * $Id: H:/drh/idioms/book/RCS/mem.doc,v 1.12 1997/10/27 23:08:05 drh Exp $
 */
#include <stdlib.h>
#include <stddef.h>
#include "assert.h"
#include "except.h"
#include "mem.h"

const except_t Mem_Failed = { "Allocation Failed" };

void *mem_alloc(long nbytes, const char *file, int line){
	void *ptr;
	assert(nbytes > 0);
	ptr = malloc(nbytes);
	if (ptr == NULL)
		{
			if (file == NULL)
				RAISE(Mem_Failed);
			else
				except_raise(&Mem_Failed, file, line);
		}
	return ptr;
}

void *mem_calloc(long count, long nbytes,
	const char *file, int line) {
	void *ptr;
	assert(count > 0);
	assert(nbytes > 0);
	ptr = calloc(count, nbytes);
	if (ptr == NULL)
		{
			if (file == NULL)
				RAISE(Mem_Failed);
			else
				except_raise(&Mem_Failed, file, line);
		}
	return ptr;
}

void mem_free(void *ptr, const char *file, int line) {
	(void)*file;
	(void)line;
	
	if (ptr)
		free(ptr);
}

void *mem_resize(void *ptr, long nbytes,
	const char *file, int line) {
	assert(ptr);
	assert(nbytes > 0);
	ptr = realloc(ptr, nbytes);
	if (ptr == NULL)
		{
			if (file == NULL)
				RAISE(Mem_Failed);
			else
				except_raise(&Mem_Failed, file, line);
		}
	return ptr;
}
