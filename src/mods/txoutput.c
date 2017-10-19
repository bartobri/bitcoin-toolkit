#include <stdlib.h>
#include <stdint.h>
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"

#define CHECK_ZERO(x)      if (x == 0) return NULL;

TXOutput txoutput_from_raw(unsigned char *raw, size_t l, size_t *c) {
	size_t i, j;
	TXOutput r;
	
	if ((r = malloc(sizeof(*r))) == NULL) {
		// TODO - handle memory error here
	}
	
	*c = 0;
	
	// Output Amount
	for (r->amount = 0, i = 0; i < sizeof(r->amount); ++i, ++raw, --l, ++(*c)) {
		CHECK_ZERO(l);
		r->amount += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	// Unlocking Script Size
	r->script_size = compactuint_get_value(raw, l, &j);
	raw += j;
	*c += j;
	l = (j > l) ? 0 : l - j;
	CHECK_ZERO(l);
	
	// Unlocking Script
	if ((r->script = malloc(r->script_size)) == NULL) {
		// TODO - Handle memory error here
	}
	for (i = 0; i < r->script_size; ++i, ++raw, --l, ++(*c)) {
		CHECK_ZERO(l);
		r->script[i] = *raw;
	}
	
	return r;
}
