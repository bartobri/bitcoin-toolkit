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

int verack_sizeof(void)
{
	return sizeof(struct Verack);
}

void verack_free(Verack v) {
	assert(v);
	
	FREE(v);
}
