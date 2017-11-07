#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"
#include "mem.h"
#include "assert.h"

Trans transaction_from_raw(unsigned char *raw, size_t l) {
	size_t i, c;
	Trans r;

	assert(raw);
	assert(l);

	NEW(r);

	// Get Version
	for (r->version = 0, i = 0; i < sizeof(r->version); ++i, ++raw, --l) {
		assert(l);
		r->version += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	// Number of Inputs
	r->input_count = compactuint_get_value(raw, l, &c);
	raw += c;
	l = (c > l) ? 0 : l - c;
	assert(l);
	
	// Input Transactions
	r->inputs = ALLOC(sizeof(TXInput) * r->input_count);
	for (i = 0; i < r->input_count; ++i) {
		r->inputs[i] = txinput_from_raw(raw, l, &c);
		raw += c;
		l = (c > l) ? 0 : l - c;
		assert(l);
	}
	
	// Number of Outputs
	r->output_count = compactuint_get_value(raw, l, &c);
	raw += c;
	l = (c > l) ? 0 : l - c;
	assert(l);
	
	// Output Transactions
	r->outputs = ALLOC(sizeof(TXOutput) * r->output_count);
	for (i = 0; i < r->output_count; ++i) {
		r->outputs[i] = txoutput_from_raw(raw, l, &c);
		raw += c;
		l = (c > l) ? 0 : l - c;
		assert(l);
	}
	
	// Lock Time
	for (r->lock_time = 0, i = 0; i < sizeof(r->lock_time); ++i, ++raw, --l) {
		assert(l);
		r->lock_time += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	return r;
}
