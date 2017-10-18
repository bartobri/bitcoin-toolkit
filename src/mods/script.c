#include <stdint.h>
#include "script.h"

typedef struct {
	const char *word;
	// placeholder for function pointer
} Words;

Words words[0xff] = {
	[0x00].word = "OP_FALSE",
	[0x01].word = "NA",
	[0x02].word = "NA",
	[0x03].word = "NA",
	[0x04].word = "NA",
	[0x05].word = "NA",
	
	[0x14].word = "NA",
	
	[0x76].word = "OP_DUP",
	
	[0x88].word = "OP_EQUALVERIFY",
	
	[0xa9].word = "OP_HASH160",
	
	[0xac].word = "OP_CHECKSIG",
};

const char *script_get_word(uint8_t w) {
	return words[w].word;
}
