#ifndef KEYPAIR_H
#define KEYPAIR_H 1

#include <gmp.h>
#include "point.h"

#define PRIVATE_KEY_BYTES            32
#define PRIVATE_KEY_COMPRESSED_BYTES 33
#define PUBLIC_KEY_BYTES             65
#define PUBLIC_KEY_COMPRESSED_BYTES  33

typedef struct {
	unsigned char private_key[PRIVATE_KEY_BYTES];
	unsigned char private_key_compressed[PRIVATE_KEY_COMPRESSED_BYTES];
	unsigned char public_key[PUBLIC_KEY_BYTES];
	unsigned char public_key_compressed[PUBLIC_KEY_COMPRESSED_BYTES];
} KeyPair;

KeyPair keypair_new(void);
void    keypair_print_address(KeyPair *);
void    keypair_print_address_compressed(KeyPair *);
void    keypair_print_wif(KeyPair *);
void    keypair_print_wif_compressed(KeyPair *);
void    keypair_dump(KeyPair);

#endif
