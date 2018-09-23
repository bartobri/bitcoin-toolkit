#include <stdlib.h>
#include <stdint.h>
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"
#include "mem.h"
#include "assert.h"

TXOutput txoutput_from_raw(unsigned char *raw, size_t l, size_t *c) {
	int r;
	size_t i, j;
	TXOutput txoutput;
	
	assert(raw);
	assert(l);

	NEW(txoutput);
	
	*c = 0;
	
	// Output Amount
	for (txoutput->amount = 0, i = 0; i < sizeof(txoutput->amount); ++i, ++raw, --l, ++(*c)) {
		assert(l);
		txoutput->amount += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	// Unlocking Script Size
	r = compactuint_get_value(&txoutput->script_size, raw, l);
	if (r < 0)
	{
		// return a negative value
	}
	j = r; // quick fix - make prettier later
	raw += j;
	*c += j;
	l = (j > l) ? 0 : l - j;
	assert(l);
	
	// Unlocking Script
	txoutput->script_raw = ALLOC(txoutput->script_size);
	for (i = 0; i < txoutput->script_size; ++i, ++raw, --l, ++(*c)) {
		assert(l);
		txoutput->script_raw[i] = *raw;
	}
	
	return txoutput;
}
