/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Copyright (c) 2023 Brian Barto
 * 
 * Modified for Bitcoin Toolkit
 */

#ifndef RMD160_H
#define RMD160_H

#include <stddef.h>

#define RIPEMD160_DIGEST_LENGTH 20
#define RIPEMD160_LONG          unsigned int
#define RIPEMD160_CBLOCK        64
#define RIPEMD160_LBLOCK        (RIPEMD160_CBLOCK/4)

typedef struct RIPEMD160state_st {
	RIPEMD160_LONG A, B, C, D, E;
	RIPEMD160_LONG Nl, Nh;
	RIPEMD160_LONG data[RIPEMD160_LBLOCK];
	unsigned int num;
} RIPEMD160_CTX;

int RIPEMD160_Init(RIPEMD160_CTX *c);
int RIPEMD160_Update(RIPEMD160_CTX *c, const void *data, size_t len);
int RIPEMD160_Final(unsigned char *md, RIPEMD160_CTX *c);
//OSSL_DEPRECATEDIN_3_0 unsigned char *RIPEMD160(const unsigned char *d, size_t n, unsigned char *md);
//OSSL_DEPRECATEDIN_3_0 void RIPEMD160_Transform(RIPEMD160_CTX *c, const unsigned char *b);

#endif
