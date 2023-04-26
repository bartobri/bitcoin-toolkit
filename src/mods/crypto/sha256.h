/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef SHA256_H
#define SHA256_H

# include <stddef.h>

#define SHA_LBLOCK      16
#define SHA256_DIGEST_LENGTH    32
#define SHA256_CBLOCK   (SHA_LBLOCK*4)/* SHA-256 treats input data as a
										* contiguous array of 32 bit wide
										* big-endian values. */

typedef struct SHA256state_st {
	unsigned int h[8];
	unsigned int Nl, Nh;
	unsigned int data[SHA_LBLOCK];
	unsigned int num;
	unsigned int md_len;
} SHA256_CTX;


int SHA256_Init(SHA256_CTX *c);
int SHA256_Update(SHA256_CTX *c, const void *data, size_t len);
int SHA256_Final(unsigned char *md, SHA256_CTX *c);

#endif
