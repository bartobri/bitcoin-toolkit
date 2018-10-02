#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"
#include "compactuint.h"
#include "error.h"
#include "mem.h"
#include "assert.h"

int txinput_from_raw(TXInput txinput, unsigned char *input, size_t input_len)
{
	int r;
	size_t i, j, c;

	assert(txinput);
	assert(input);
	assert(input_len);
	
	c = 0;
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, ++input, --input_len, ++c)
	{
		if (input_len == 0)
		{
			error_log("Transaction input data is incomplete.");
			return -1;
		}
		txinput->tx_hash[31-i] = *input;
	}
	
	// Output index of transcaction hash
	for (i = 0; i < sizeof(txinput->index); ++i, ++input, --input_len, ++c)
	{
		if (input_len == 0)
		{
			error_log("Transaction input data is incomplete.");
			return -1;
		}
		txinput->index <<= 8;
		txinput->index += *input;
	}
	
	// Unlocking Script Size
	r = compactuint_get_value(&txinput->script_size, input, input_len);
	if (r < 0)
	{
		error_log("Error while parsing compact size integer from transaction input data.");
		return -1;
	}
	j = r; // quick fix - make prettier later
	input += j;
	c += j;
	input_len = (j > input_len) ? 0 : input_len - j;
	if (input_len == 0)
	{
		error_log("Transaction input data is incomplete.");
		return -1;
	}
	
	// Unlocking Script
	txinput->script_raw = ALLOC(txinput->script_size);
	for (i = 0; i < txinput->script_size; ++i, ++input, --input_len, ++c)
	{
		if (input_len == 0)
		{
			error_log("Transaction input data is incomplete.");
			return -1;
		}
		txinput->script_raw[i] = *input;
	}
	
	// Sequence
	for (i = 0; i < sizeof(txinput->sequence); ++i, ++input, --input_len, ++c)
	{
		if (input_len == 0)
		{
			error_log("Transaction input data is incomplete.");
			return -1;
		}
		txinput->sequence <<= 8;
		txinput->sequence += *input;
	}
	
	return c;
}
