#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"
#include "compactuint.h"

TXInput txinput_from_raw(unsigned char *raw, size_t *l) {
	size_t i, c;
	TXInput r;
	
	*l = 0;
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, ++raw, ++(*l)) {
		r.tx_hash[i] = *raw;
	}
	
	// Output index of transcaction hash
	for (i = 0; i < sizeof(r.index); ++i, ++raw, ++(*l)) {
		r.index <<= 8;
		r.index += *raw;
	}
	
	// Unlocking Script Size
	r.script_size = compactuint_get_value(raw, &c);
	raw += c;
	*l += c;
	
	// Unlocking Script
	if ((r.script = malloc(r.script_size)) == NULL) {
		// TODO - Handle memory error here
	}
	for (i = 0; i < r.script_size; ++i, ++raw, ++(*l)) {
		r.script[i] = *raw;
	}
	
	// Sequence
	for (i = 0; i < sizeof(r.sequence); ++i, ++raw, ++(*l)) {
		r.sequence <<= 8;
		r.sequence += *raw;
	}
	
	return r;
}
