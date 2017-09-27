#include <stdio.h>

#define RANDOM_SOURCE "/dev/urandom"

unsigned char random_get_byte(void) {
	unsigned char c;
	FILE *source;
	
	// FIXME - should not return zero upon failure
	if ((source = fopen(RANDOM_SOURCE, "r")) == NULL)
		return 0;
	
	c = fgetc(source);
		
	fclose(source);
	
	return c;
}
