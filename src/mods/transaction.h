#ifndef TRANSACTION_H
#define TRANSACTION_H 1

#include <stdint.h>
#include "txinput.h"
#include "txoutput.h"

typedef struct {
	uint32_t  version;
	uint64_t  input_count;
	TXInput  *inputs;
	uint64_t  output_count;
	TXOutput *outputs;
	uint32_t  lock_time;
} *Trans;

Trans transaction_from_raw(unsigned char *, size_t);

#endif
