#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"
#include "compactuint.h"

TXInput txinput_from_rawhex(char *hex) {
	size_t i, c;
	TXInput r;
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, hex += 2) {
		r.tx_hash[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	// Output index of transcaction hash
	r.index = (uint32_t)hex_to_dec_substr(0, hex, 8);
	hex += 8;
	
	// Unlocking Script Size
	r.script_size = compactuint_get_value(hex, &c);
	hex += c;
	
	// Unlocking Script
	if ((r.script = malloc(r.script_size)) == NULL) {
		// TODO - Handle memory error here
	}
	for (i = 0; i < r.script_size; ++i, hex += 2) {
		r.script[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	// Sequence
	r.sequence = (uint32_t)hex_to_dec_substr(0, hex, 8);
	
	return r;
}
