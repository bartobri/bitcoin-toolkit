#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "transaction.h"
#include "txinput.h"
#include "txoutput.h"
#include "hex.h"
#include "compactuint.h"
#include "error.h"

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
			error_log("Transaction data is incomplete.");
			return -1;
		}
		trans->version += (((size_t)*input) << (i * 8)); // Reverse byte order
	}
	
	// Number of Inputs
	r = compactuint_get_value(&trans->input_count, input, input_len);
	if (r < 0)
	{
		error_log("Could not parse compact size integer from transaction data.");
		return -1;
	}
	c = r; // quick fix - make prettier later
	input += c;
	input_len = (c > input_len) ? 0 : input_len - c;
	if (input_len == 0)
	{
		error_log("Transaction data is incomplete.");
		return -1;
	}
	
	// Input Transactions
	trans->inputs = malloc(sizeof(TXInput) * trans->input_count);
	if (trans->inputs == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	for (i = 0; i < trans->input_count; ++i)
	{
		r = txinput_from_raw(trans->inputs[i], input, input_len);
		if (r < 0)
		{
			error_log("Could not parse transaction inputs.");
			return -1;
		}
		c = r; // quick fix - make prettier later
		input += c;
		input_len = (c > input_len) ? 0 : input_len - c;
		if (input_len == 0)
		{
			error_log("Transaction data is incomplete.");
			return -1;
		}
	}
	
	// Number of Outputs
	r = compactuint_get_value(&trans->output_count, input, input_len);
	if (r < 0)
	{
		error_log("Could not parse compact size integer from transaction data.");
		return -1;
	}
	c = r; // quick fix - make prettier later
	input += c;
	input_len = (c > input_len) ? 0 : input_len - c;
	if (input_len == 0)
	{
		error_log("Transaction data is incomplete.");
		return -1;
	}
	
	// Output Transactions
	trans->outputs = malloc(sizeof(TXOutput) * trans->output_count);
	if (trans->outputs == NULL)
	{
		error_log("Memory allocation error.");
		return -1;
	}
	for (i = 0; i < trans->output_count; ++i)
	{
		r = txoutput_from_raw(trans->outputs[i], input, input_len);
		if (r < 0)
		{
			error_log("Could not parse transaction outputs.");
			return -1;
		}
		c = r;
		input += c;
		input_len = (c > input_len) ? 0 : input_len - c;
		if (input_len == 0)
		{
			error_log("Transaction data is incomplete.");
			return -1;
		}
	}
	
	// Lock Time
	for (trans->lock_time = 0, i = 0; i < sizeof(trans->lock_time); ++i, ++input, --input_len)
	{
		if (input_len == 0)
		{
			error_log("Transaction data is incomplete.");
			return -1;
		}
		trans->lock_time += (((size_t)*input) << (i * 8)); // Reverse byte order
	}
	
	return 1;
}
