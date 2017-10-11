#include <stdlib.h>
#include <stdint.h>
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"

TXOutput txoutput_from_rawhex(char *hex) {
	size_t i, c;
	TXOutput r;
	
	// Output index of transcaction hash
	r.amount = hex_to_dec_substr(0, hex, 16);
	hex += 16;
	
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
	
	return r;
}
