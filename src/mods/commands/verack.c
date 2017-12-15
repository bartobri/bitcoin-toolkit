#include "verack.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"


struct Verack {
	// nothing here
};

Verack verack_new(void) {
	Verack r;
	
	NEW(r);
	
	return r;
}

void verack_free(Verack v) {
	assert(v);
	
	FREE(v);
}
