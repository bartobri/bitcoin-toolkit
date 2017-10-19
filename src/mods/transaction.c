#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"

// TODO - Check for mal-formed transactions
Trans transaction_from_raw(unsigned char *raw, size_t l) {
	size_t i, c;
	Trans r;

	if ((r = malloc(sizeof(*r))) == NULL) {
		return NULL;
	}

	// Get Version
	for (r->version = 0, i = 0; i < sizeof(r->version); ++i, ++raw, --l) {
		if (l == 0) {
			return NULL;
		}
		r->version += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	// Unlocking Script Size
	r->input_count = compactuint_get_value(raw, &c);
	raw += c;
	l = (c > l) ? 0 : l - c;
	if (l == 0) {
		return NULL;
	}
	
	// Input Transactions
	r->inputs = malloc(sizeof(TXInput) * r->input_count);
	if (r->inputs == NULL) {
		// TODO - handle memory error
	}
	for (i = 0; i < r->input_count; ++i) {
		r->inputs[i] = txinput_from_raw(raw, &c);
		raw += c;
		l = (c > l) ? 0 : l - c;
		if (l == 0) {
			return NULL;
		}
	}
	
	// Unlocking Script Size
	r->output_count = compactuint_get_value(raw, &c);
	raw += c;
	l = (c > l) ? 0 : l - c;
	if (l == 0) {
		return NULL;
	}
	
	// Output Transactions
	r->outputs = malloc(sizeof(TXOutput) * r->output_count);
	if (r->outputs == NULL) {
		// TODO - handle memory error
	}
	for (i = 0; i < r->output_count; ++i) {
		r->outputs[i] = txoutput_from_raw(raw, &c);
		raw += c;
		l = (c > l) ? 0 : l - c;
		if (l == 0) {
			return NULL;
		}
	}
	
	// Lock Time
	for (r->lock_time = 0, i = 0; i < sizeof(r->lock_time); ++i, ++raw, --l) {
		if (l == 0) {
			return NULL;
		}
		r->lock_time += (((size_t)*raw) << (i * 8)); // Reverse byte order
	}
	
	return r;
}