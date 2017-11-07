#include <stdlib.h>
#include <stdint.h>
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"
#include "mem.h"
#include "assert.h"

//#define CHECK_ZERO(x)      if (x == 0) return NULL;

TXOutput txoutput_from_raw(unsigned char *raw, size_t l, size_t *c) {
	size_t i, j;
	TXOutput r;
	
	assert(raw);
	assert(l);

	NEW(r);
	
	*c = 0;
	
	// Output Amount
	for (r->amount = 0, i = 0; i < sizeof(r->amount); ++i, ++raw, --l, ++(*c)) {
		assert(l);
		r->amount += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	// Unlocking Script Size
	r->script_size = compactuint_get_value(raw, l, &j);
	raw += j;
	*c += j;
	l = (j > l) ? 0 : l - j;
	assert(l);
	
	// Unlocking Script
	r->script_raw = ALLOC(r->script_size);
	for (i = 0; i < r->script_size; ++i, ++raw, --l, ++(*c)) {
		assert(l);
		r->script_raw[i] = *raw;
	}
	
	return r;
}
