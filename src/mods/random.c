#include <stdio.h>
#include "assert.h"

#define RANDOM_SOURCE "/dev/urandom"

int random_get(unsigned char *output, size_t bytes)
{
	size_t i;
	int c;
	FILE *source;

	assert(output);
	assert(bytes);

	source = fopen(RANDOM_SOURCE, "r");
	if (source == NULL)
	{
		return -1;
	}

	for (i = 0; i < bytes; ++i)
	{
		c = fgetc(source);
		if (c == EOF)
		{
			return -1;
		}

		output[i] = c;
	}

	fclose(source);

	return 1;
}
