#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"
#include "compactuint.h"
#include "mem.h"

#define CHECK_ZERO(x)      if (x == 0) return NULL;

TXInput txinput_from_raw(unsigned char *raw, size_t l, size_t *c) {
	size_t i, j;
	TXInput r;

	NEW(r);
	
	*c = 0;
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, ++raw, --l, ++(*c)) {
		CHECK_ZERO(l);
		r->tx_hash[31-i] = *raw;
	}
	
	// Output index of transcaction hash
	for (i = 0; i < sizeof(r->index); ++i, ++raw, --l, ++(*c)) {
		CHECK_ZERO(l);
		r->index <<= 8;
		r->index += *raw;
	}
	
	// Unlocking Script Size
	r->script_size = compactuint_get_value(raw, l, &j);
	CHECK_ZERO(r->script_size);
	raw += j;
	*c += j;
	l = (j > l) ? 0 : l - j;
	CHECK_ZERO(l);
	
	// Unlocking Script
	r->script_raw = ALLOC(r->script_size);
	for (i = 0; i < r->script_size; ++i, ++raw, --l, ++(*c)) {
		CHECK_ZERO(l);
		r->script_raw[i] = *raw;
	}
	
	// Sequence
	for (i = 0; i < sizeof(r->sequence); ++i, ++raw, --l, ++(*c)) {
		CHECK_ZERO(l);
		r->sequence <<= 8;
		r->sequence += *raw;
	}
	
	return r;
}
