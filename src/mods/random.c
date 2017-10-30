#include <stdio.h>
#include "assert.h"

#define RANDOM_SOURCE "/dev/urandom"

unsigned char random_get(void) {
	int c;
	FILE *source;

	source = fopen(RANDOM_SOURCE, "r");
	
	assert(source);

	c = fgetc(source);

	fclose(source);
	
	assert(c >= 0);

	return (unsigned char)c;
}
