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
unsigned char *version_serialize(Version v) {
	return NULL;
}
