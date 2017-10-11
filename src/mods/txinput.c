#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"
#include "compactuint.h"

TXInput txinput_from_rawhex(char *hex) {
	size_t i, c;
	TXInput r;
	char tx_index[9];
	char sequence[9];
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, hex += 2) {
		r.tx_hash[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	// Output index of transcaction hash
	for (i = 0; i < 8; ++i, ++hex) {
		tx_index[i] = hex[0];
	}
	tx_index[i] = '\n';
	r.index = (uint32_t)strtoul(tx_index, NULL, 16);
	
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
	for (i = 0; i < 8; ++i, ++hex) {
		sequence[i] = hex[0];
	}
	sequence[i] = '\0';
	r.sequence = (uint32_t)strtoul(sequence, NULL, 16);
	
	return r;
}
