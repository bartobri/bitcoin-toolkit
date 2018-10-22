#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"
#include "error.h"

int txoutput_from_raw(TXOutput txoutput, unsigned char *input, size_t input_len)
{
	int r;
	size_t i, j, c;
	
	assert(txoutput);
	assert(input);
	assert(input_len);
	
	c = 0;
	
	// Output Amount
	for (txoutput->amount = 0, i = 0; i < sizeof(txoutput->amount); ++i, ++input, --input_len, ++c)
	{
		if (input_len == 0)
		{
			error_log("Transaction output data is incomplete.");
			return -1;
		}
		txoutput->amount += (((size_t)*input) << (i * 8)); // Reverse byte order
	}
	
	// Unlocking Script Size
	r = compactuint_get_value(&txoutput->script_size, input, input_len);
	if (r < 0)
	{
		error_log("Could not parse compact size integer from transaction output data.");
		return -1;
	}
	j = r; // quick fix - make prettier later
	input += j;
	c += j;
	input_len = (j > input_len) ? 0 : input_len - j;
	if (input_len == 0)
	{
		error_log("Transaction output data is incomplete.");
		return -1;
	}
	
	// Unlocking Script
	txoutput->script_raw = malloc(txoutput->script_size);
	if (txoutput->script_raw == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	for (i = 0; i < txoutput->script_size; ++i, ++input, --input_len, ++c)
	{
		if (input_len == 0)
		{
			error_log("Transaction output data is incomplete.");
			return -1;
		}
		txoutput->script_raw[i] = *input;
	}
	
	return (int)c;
}
