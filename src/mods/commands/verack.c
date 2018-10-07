#include <stddef.h>
#include "verack.h"


struct Verack {
	// nothing here
};

int verack_new(Verack v) {
	(void)v;

	return 1;
}

size_t verack_sizeof(void)
{
	return sizeof(struct Verack);
}
