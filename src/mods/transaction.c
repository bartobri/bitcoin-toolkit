#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"
#include "mem.h"
#include "assert.h"

int transaction_from_raw(Trans trans, unsigned char *input, size_t input_len)
{
	int r;
	size_t i, c;

	assert(trans);
	assert(input);
	assert(input_len);

	// Get Version
	for (trans->version = 0, i = 0; i < sizeof(trans->version); ++i, ++input, --input_len)
	{
		if (input_len == 0)
		{
			return -1;
		}
		trans->version += (((size_t)*input) << (i * 8)); // Reverse byte order
	}
	
	// Number of Inputs
	r = compactuint_get_value(&trans->input_count, input, input_len);
	if (r < 0)
	{
		return -1;
	}
	c = r; // quick fix - make prettier later
	input += c;
	input_len = (c > input_len) ? 0 : input_len - c;
	if (input_len == 0)
	{
		return -1;
	}
	
	// Input Transactions
	trans->inputs = ALLOC(sizeof(TXInput) * trans->input_count);
	for (i = 0; i < trans->input_count; ++i)
	{
		trans->inputs[i] = txinput_from_raw(input, input_len, &c);
		input += c;
		input_len = (c > input_len) ? 0 : input_len - c;
		if (input_len == 0)
		{
			return -1;
		}
	}
	
	// Number of Outputs
	r = compactuint_get_value(&trans->output_count, input, input_len);
	if (r < 0)
	{
		return -1;
	}
	c = r; // quick fix - make prettier later
	input += c;
	input_len = (c > input_len) ? 0 : input_len - c;
	if (input_len == 0)
	{
		return -1;
	}
	
	// Output Transactions
	trans->outputs = ALLOC(sizeof(TXOutput) * trans->output_count);
	for (i = 0; i < trans->output_count; ++i)
	{
		trans->outputs[i] = txoutput_from_raw(input, input_len, &c);
		input += c;
		input_len = (c > input_len) ? 0 : input_len - c;
		if (input_len == 0)
		{
			return -1;
		}
	}
	
	// Lock Time
	for (trans->lock_time = 0, i = 0; i < sizeof(trans->lock_time); ++i, ++input, --input_len)
	{
		if (input_len == 0)
		{
			return -1;
		}
		trans->lock_time += (((size_t)*input) << (i * 8)); // Reverse byte order
	}
	
	return 1;
}
