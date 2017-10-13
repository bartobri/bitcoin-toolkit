#ifndef TXOUTPUT_H
#define TXOUTPUT_H 1

#include <stdint.h>

typedef struct {
	uint64_t       amount;
	uint64_t       script_size;
	unsigned char* script;
} TXOutput;

TXOutput txoutput_from_raw(unsigned char *, size_t *);

#endif
