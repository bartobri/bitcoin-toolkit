#ifndef TXINPUT_H
#define TXINPUT_H 1

#include <stdint.h>

typedef struct TXInput *TXInput;
struct TXInput {
	unsigned char  tx_hash[32];
	uint32_t       index;
	uint64_t       script_size;
	unsigned char* script_raw;
	uint32_t       sequence;
};

TXInput txinput_from_raw(unsigned char *, size_t, size_t *);

#endif
