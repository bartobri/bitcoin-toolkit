#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"

// TODO - validate string is fully hexidecimal and even numbered
unsigned char *transaction_hex_to_raw(char *hex) {
	size_t i, l;
	unsigned char *raw;
	
	l = strlen(hex);
	
	if ((raw = malloc(l / 2)) == NULL) {
		// TODO - handle memory error
	}
	
	for (i = 0; i < l; ++i, hex += 2) {
		raw[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	return raw;
}

Trans transaction_from_rawhex(char *hex) {
	size_t i, c;
	Trans r;

	// Get Version
	r.version = (uint32_t)hex_to_dec_substr(0, hex, 8);
	hex += 8;
	
	// Unlocking Script Size
	r.input_count = compactuint_get_value(hex, &c);
	hex += c;
	
	// Input Transactions
	r.inputs = malloc(sizeof(TXInput) * r.input_count);
	if (r.inputs == NULL) {
		// TODO - handle memory error
	}
	for (i = 0; i < r.input_count; ++i) {
		r.inputs[i] = txinput_from_rawhex(hex, &c);
		hex += c;
	}
	
	// Unlocking Script Size
	r.output_count = compactuint_get_value(hex, &c);
	hex += c;
	
	// Output Transactions
	r.outputs = malloc(sizeof(TXOutput) * r.output_count);
	if (r.outputs == NULL) {
		// TODO - handle memory error
	}
	for (i = 0; i < r.output_count; ++i) {
		r.outputs[i] = txoutput_from_rawhex(hex, &c);
		hex += c;
	}
	
	// Lock Time
	r.lock_time = (uint32_t)hex_to_dec_substr(0, hex, 8);
	
	return r;
}
