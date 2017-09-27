#include <stdio.h>

#define RANDOM_SOURCE "/dev/urandom"

int random_get(void) {
	int c;
	FILE *source;

	if ((source = fopen(RANDOM_SOURCE, "r")) == NULL)
		return -1;

	c = fgetc(source);

	fclose(source);

	return c;
}
