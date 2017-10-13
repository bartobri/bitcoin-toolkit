#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"

Trans transaction_from_raw(unsigned char *raw) {
	size_t i, c;
	Trans r;

	// Get Version
	for (r.version = 0, i = 0; i < sizeof(r.version); ++i, ++raw) {
		r.version <<= 8;
		r.version += *raw;
	}
	
	// Unlocking Script Size
	r.input_count = compactuint_get_value(raw, &c);
	raw += c;
	
	// Input Transactions
	r.inputs = malloc(sizeof(TXInput) * r.input_count);
	if (r.inputs == NULL) {
		// TODO - handle memory error
	}
	for (i = 0; i < r.input_count; ++i) {
		r.inputs[i] = txinput_from_raw(raw, &c);
		raw += c;
	}
	
	// Unlocking Script Size
	r.output_count = compactuint_get_value(raw, &c);
	raw += c;
	
	// Output Transactions
	r.outputs = malloc(sizeof(TXOutput) * r.output_count);
	if (r.outputs == NULL) {
		// TODO - handle memory error
	}
	for (i = 0; i < r.output_count; ++i) {
		r.outputs[i] = txoutput_from_raw(raw, &c);
		raw += c;
	}
	
	// Lock Time
	for (i = 0; i < sizeof(r.lock_time); ++i, ++raw) {
		r.lock_time <<= 8;
		r.lock_time += *raw;
	}
	
	return r;
}
