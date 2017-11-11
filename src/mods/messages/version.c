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
