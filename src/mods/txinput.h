#ifndef TXINPUT_H
#define TXINPUT_H 1

#include <stdint.h>

typedef struct {
	unsigned char  tx_hash[32];
	uint32_t       index;
	uint64_t       script_size;
	unsigned char* script;
	uint32_t       sequence;
} *TXInput;

TXInput txinput_from_raw(unsigned char *, size_t *);

#endif
