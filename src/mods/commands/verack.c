#include "verack.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"


struct Verack {
	// nothing here
};

int verack_new(Verack v) {
	(void)v;

	return 1;
}

void verack_free(Verack v) {
	assert(v);
	
	FREE(v);
}
