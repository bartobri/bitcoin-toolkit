#include <stddef.h>
#include "version.h"
#include "mods/mem.h"
#include "mods/assert.h"

struct Version {
	int temp;
};

Version version_new(void) {
	Version r;
	
	NEW(r);
	
	return r;
}

// TODO - need to also implement a length argument
size_t version_serialize(Version v, unsigned char **s) {
	size_t len;
	unsigned char *temp;
	
	len = 10;
	
	temp = ALLOC(len);
	
	memset(temp, 5, len);
	
	*s = temp;

	return len;
}
