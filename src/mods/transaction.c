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
	int r;
	size_t i, c;
	Trans trans;

	assert(raw);
	assert(l);

	NEW(trans);

	// Get Version
	for (trans->version = 0, i = 0; i < sizeof(trans->version); ++i, ++raw, --l) {
		assert(l);
		trans->version += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	// Number of Inputs
	r = compactuint_get_value(&trans->input_count, raw, l);
	if (r < 0)
	{
		// return a negative value
	}
	c = r; // quick fix - make prettier later
	raw += c;
	l = (c > l) ? 0 : l - c;
	assert(l);
	
	// Input Transactions
	trans->inputs = ALLOC(sizeof(TXInput) * trans->input_count);
	for (i = 0; i < trans->input_count; ++i) {
		trans->inputs[i] = txinput_from_raw(raw, l, &c);
		raw += c;
		l = (c > l) ? 0 : l - c;
		assert(l);
	}
	
	// Number of Outputs
	r = compactuint_get_value(&trans->output_count, raw, l);
	if (r < 0)
	{
		// return a negative value
	}
	c = r; // quick fix - make prettier later
	raw += c;
	l = (c > l) ? 0 : l - c;
	assert(l);
	
	// Output Transactions
	trans->outputs = ALLOC(sizeof(TXOutput) * trans->output_count);
	for (i = 0; i < trans->output_count; ++i) {
		trans->outputs[i] = txoutput_from_raw(raw, l, &c);
		raw += c;
		l = (c > l) ? 0 : l - c;
		assert(l);
	}
	
	// Lock Time
	for (trans->lock_time = 0, i = 0; i < sizeof(trans->lock_time); ++i, ++raw, --l) {
		assert(l);
		trans->lock_time += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	return trans;
}
