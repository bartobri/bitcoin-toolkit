#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"
#include "compactuint.h"
#include "mem.h"
#include "assert.h"

TXInput txinput_from_raw(unsigned char *raw, size_t l, size_t *c) {
	int r;
	size_t i, j;
	TXInput txinput;

	assert(raw);
	assert(l);

	NEW(txinput);
	
	*c = 0;
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, ++raw, --l, ++(*c)) {
		assert(l);
		txinput->tx_hash[31-i] = *raw;
	}
	
	// Output index of transcaction hash
	for (i = 0; i < sizeof(txinput->index); ++i, ++raw, --l, ++(*c)) {
		assert(l);
		txinput->index <<= 8;
		txinput->index += *raw;
	}
	
	// Unlocking Script Size
	r = compactuint_get_value(&txinput->script_size, raw, l);
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
	txinput->script_raw = ALLOC(txinput->script_size);
	for (i = 0; i < txinput->script_size; ++i, ++raw, --l, ++(*c)) {
		assert(l);
		txinput->script_raw[i] = *raw;
	}
	
	// Sequence
	for (i = 0; i < sizeof(txinput->sequence); ++i, ++raw, --l, ++(*c)) {
		assert(l);
		txinput->sequence <<= 8;
		txinput->sequence += *raw;
	}
	
	return txinput;
}
