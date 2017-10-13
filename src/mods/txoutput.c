#include <stdlib.h>
#include <stdint.h>
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"

TXOutput txoutput_from_raw(unsigned char *raw, size_t *l) {
	size_t i, c;
	TXOutput r;
	
	*l = 0;
	
	// Output index of transcaction hash
	for (r.amount = 0, i = 0; i < sizeof(r.amount); ++i, ++raw, ++(*l)) {
		r.amount <<= 8;
		r.amount += *raw;
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
	
	return r;
}
