/*
 * Copyright 2004-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * SHA256 low level APIs are deprecated for public use, but still ok for
 * internal use.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256.h"

#define IS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#define DATA_ORDER_IS_BIG_ENDIAN

#if defined(DATA_ORDER_IS_BIG_ENDIAN)

#define HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))<<24),          \
						 l|=(((unsigned long)(*((c)++)))<<16),          \
						 l|=(((unsigned long)(*((c)++)))<< 8),          \
						 l|=(((unsigned long)(*((c)++)))    )           )
#define HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)>>24)&0xff),      \
						 *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
						 *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
						 *((c)++)=(unsigned char)(((l)    )&0xff),      \
						 l)

#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)

#define HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))    ),          \
						 l|=(((unsigned long)(*((c)++)))<< 8),          \
						 l|=(((unsigned long)(*((c)++)))<<16),          \
						 l|=(((unsigned long)(*((c)++)))<<24)           )
#define HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)    )&0xff),      \
						 *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
						 *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
						 *((c)++)=(unsigned char)(((l)>>24)&0xff),      \
						 l)
#endif

#define HASH_MAKE_STRING(c,s)   do {    \
		unsigned long ll;               \
		unsigned int  nn;               \
		for (nn=0;nn<SHA256_DIGEST_LENGTH/4;nn++)       \
		{   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
		} while (0)

#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))

static const unsigned int K256[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
	0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
	0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
	0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
	0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
	0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
	0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
	0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
	0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
	0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
	0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
	0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
	0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/*
 * FIPS specification refers to right rotations, while our ROTATE macro
 * is left one. This is why you might notice that rotation coefficients
 * differ from those observed in FIPS document by 32-N...
 */
#  define Sigma0(x)       (ROTATE((x),30) ^ ROTATE((x),19) ^ ROTATE((x),10))
#  define Sigma1(x)       (ROTATE((x),26) ^ ROTATE((x),21) ^ ROTATE((x),7))
#  define sigma0(x)       (ROTATE((x),25) ^ ROTATE((x),14) ^ ((x)>>3))
#  define sigma1(x)       (ROTATE((x),15) ^ ROTATE((x),13) ^ ((x)>>10))
#  define Ch(x,y,z)       (((x) & (y)) ^ ((~(x)) & (z)))
#  define Maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#  define ROUND_00_15(i,a,b,c,d,e,f,g,h)          do {    \
		T1 += h + Sigma1(e) + Ch(e,f,g) + K256[i];      \
		h = Sigma0(a) + Maj(a,b,c);                     \
		d += T1;        h += T1;                } while (0)

#  define ROUND_16_63(i,a,b,c,d,e,f,g,h,X)        do {    \
		s0 = X[(i+1)&0x0f];     s0 = sigma0(s0);        \
		s1 = X[(i+14)&0x0f];    s1 = sigma1(s1);        \
		T1 = X[(i)&0x0f] += s0 + s1 + X[(i+9)&0x0f];    \
		ROUND_00_15(i,a,b,c,d,e,f,g,h);         } while (0)

void sha256_block_data_order(SHA256_CTX *ctx, const void *in, size_t num);
void sha256_cleanse(void *ptr, size_t len);

typedef void *(*memset_t)(void *, int, size_t);
static volatile memset_t memset_func = memset;

int SHA256_Init(SHA256_CTX *c)
{
	memset(c, 0, sizeof(*c));
	c->h[0] = 0x6a09e667UL;
	c->h[1] = 0xbb67ae85UL;
	c->h[2] = 0x3c6ef372UL;
	c->h[3] = 0xa54ff53aUL;
	c->h[4] = 0x510e527fUL;
	c->h[5] = 0x9b05688cUL;
	c->h[6] = 0x1f83d9abUL;
	c->h[7] = 0x5be0cd19UL;
	c->md_len = SHA256_DIGEST_LENGTH;
	return 1;
}

int SHA256_Update(SHA256_CTX *c, const void *data_, size_t len)
{
	const unsigned char *data = data_;
	unsigned char *p;
	unsigned int l;
	size_t n;

	if (len == 0)
		return 1;

	l = (c->Nl + (((unsigned int) len) << 3)) & 0xffffffffUL;
	if (l < c->Nl)              /* overflow */
		c->Nh++;
	c->Nh += (unsigned int) (len >> 29); /* might cause compiler warning on 16-bit */
	c->Nl = l;

	n = c->num;
	if (n != 0) {
		p = (unsigned char *)c->data;

		if (len >= SHA256_CBLOCK || len + n >= SHA256_CBLOCK) {
			memcpy(p + n, data, SHA256_CBLOCK - n);
			sha256_block_data_order(c, p, 1);
			n = SHA256_CBLOCK - n;
			data += n;
			len -= n;
			c->num = 0;
			/*
			 * We use memset rather than OPENSSL_cleanse() here deliberately.
			 * Using OPENSSL_cleanse() here could be a performance issue. It
			 * will get properly cleansed on finalisation so this isn't a
			 * security problem.
			 */
			memset(p, 0, SHA256_CBLOCK); /* keep it zeroed */
		} else {
			memcpy(p + n, data, len);
			c->num += (unsigned int)len;
			return 1;
		}
	}

	n = len / SHA256_CBLOCK;
	if (n > 0) {
		sha256_block_data_order(c, data, n);
		n *= SHA256_CBLOCK;
		data += n;
		len -= n;
	}

	if (len != 0) {
		p = (unsigned char *)c->data;
		c->num = (unsigned int)len;
		memcpy(p, data, len);
	}
	return 1;
}

int SHA256_Final(unsigned char *md, SHA256_CTX *c)
{
	unsigned char *p = (unsigned char *)c->data;
	size_t n = c->num;

	p[n] = 0x80;                /* there is always room for one */
	n++;

	if (n > (SHA256_CBLOCK - 8)) {
		memset(p + n, 0, SHA256_CBLOCK - n);
		n = 0;
		sha256_block_data_order(c, p, 1);
	}
	memset(p + n, 0, SHA256_CBLOCK - 8 - n);

	p += SHA256_CBLOCK - 8;
# if   defined(DATA_ORDER_IS_BIG_ENDIAN)
	(void)HOST_l2c(c->Nh, p);
	(void)HOST_l2c(c->Nl, p);
# elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
	(void)HOST_l2c(c->Nl, p);
	(void)HOST_l2c(c->Nh, p);
# endif
	p -= SHA256_CBLOCK;
	sha256_block_data_order(c, p, 1);
	c->num = 0;

	sha256_cleanse(p, SHA256_CBLOCK);

	HASH_MAKE_STRING(c, md);

	return 1;
}

void sha256_block_data_order(SHA256_CTX *ctx, const void *in, size_t num)
{
	unsigned int a, b, c, d, e, f, g, h, s0, s1, T1;
	unsigned int X[16];
	int i;
	const unsigned char *data = in;

	while (num--) {

		a = ctx->h[0];
		b = ctx->h[1];
		c = ctx->h[2];
		d = ctx->h[3];
		e = ctx->h[4];
		f = ctx->h[5];
		g = ctx->h[6];
		h = ctx->h[7];

		if (!IS_LITTLE_ENDIAN && sizeof(unsigned int) == 4 && ((size_t)in % 4) == 0) {
			const unsigned int *W = (const unsigned int *)data;

			T1 = X[0] = W[0];
			ROUND_00_15(0, a, b, c, d, e, f, g, h);
			T1 = X[1] = W[1];
			ROUND_00_15(1, h, a, b, c, d, e, f, g);
			T1 = X[2] = W[2];
			ROUND_00_15(2, g, h, a, b, c, d, e, f);
			T1 = X[3] = W[3];
			ROUND_00_15(3, f, g, h, a, b, c, d, e);
			T1 = X[4] = W[4];
			ROUND_00_15(4, e, f, g, h, a, b, c, d);
			T1 = X[5] = W[5];
			ROUND_00_15(5, d, e, f, g, h, a, b, c);
			T1 = X[6] = W[6];
			ROUND_00_15(6, c, d, e, f, g, h, a, b);
			T1 = X[7] = W[7];
			ROUND_00_15(7, b, c, d, e, f, g, h, a);
			T1 = X[8] = W[8];
			ROUND_00_15(8, a, b, c, d, e, f, g, h);
			T1 = X[9] = W[9];
			ROUND_00_15(9, h, a, b, c, d, e, f, g);
			T1 = X[10] = W[10];
			ROUND_00_15(10, g, h, a, b, c, d, e, f);
			T1 = X[11] = W[11];
			ROUND_00_15(11, f, g, h, a, b, c, d, e);
			T1 = X[12] = W[12];
			ROUND_00_15(12, e, f, g, h, a, b, c, d);
			T1 = X[13] = W[13];
			ROUND_00_15(13, d, e, f, g, h, a, b, c);
			T1 = X[14] = W[14];
			ROUND_00_15(14, c, d, e, f, g, h, a, b);
			T1 = X[15] = W[15];
			ROUND_00_15(15, b, c, d, e, f, g, h, a);

			data += SHA256_CBLOCK;
		} else {
			unsigned int l;

			(void)HOST_c2l(data, l);
			T1 = X[0] = l;
			ROUND_00_15(0, a, b, c, d, e, f, g, h);
			(void)HOST_c2l(data, l);
			T1 = X[1] = l;
			ROUND_00_15(1, h, a, b, c, d, e, f, g);
			(void)HOST_c2l(data, l);
			T1 = X[2] = l;
			ROUND_00_15(2, g, h, a, b, c, d, e, f);
			(void)HOST_c2l(data, l);
			T1 = X[3] = l;
			ROUND_00_15(3, f, g, h, a, b, c, d, e);
			(void)HOST_c2l(data, l);
			T1 = X[4] = l;
			ROUND_00_15(4, e, f, g, h, a, b, c, d);
			(void)HOST_c2l(data, l);
			T1 = X[5] = l;
			ROUND_00_15(5, d, e, f, g, h, a, b, c);
			(void)HOST_c2l(data, l);
			T1 = X[6] = l;
			ROUND_00_15(6, c, d, e, f, g, h, a, b);
			(void)HOST_c2l(data, l);
			T1 = X[7] = l;
			ROUND_00_15(7, b, c, d, e, f, g, h, a);
			(void)HOST_c2l(data, l);
			T1 = X[8] = l;
			ROUND_00_15(8, a, b, c, d, e, f, g, h);
			(void)HOST_c2l(data, l);
			T1 = X[9] = l;
			ROUND_00_15(9, h, a, b, c, d, e, f, g);
			(void)HOST_c2l(data, l);
			T1 = X[10] = l;
			ROUND_00_15(10, g, h, a, b, c, d, e, f);
			(void)HOST_c2l(data, l);
			T1 = X[11] = l;
			ROUND_00_15(11, f, g, h, a, b, c, d, e);
			(void)HOST_c2l(data, l);
			T1 = X[12] = l;
			ROUND_00_15(12, e, f, g, h, a, b, c, d);
			(void)HOST_c2l(data, l);
			T1 = X[13] = l;
			ROUND_00_15(13, d, e, f, g, h, a, b, c);
			(void)HOST_c2l(data, l);
			T1 = X[14] = l;
			ROUND_00_15(14, c, d, e, f, g, h, a, b);
			(void)HOST_c2l(data, l);
			T1 = X[15] = l;
			ROUND_00_15(15, b, c, d, e, f, g, h, a);
		}

		for (i = 16; i < 64; i += 8) {
			ROUND_16_63(i + 0, a, b, c, d, e, f, g, h, X);
			ROUND_16_63(i + 1, h, a, b, c, d, e, f, g, X);
			ROUND_16_63(i + 2, g, h, a, b, c, d, e, f, X);
			ROUND_16_63(i + 3, f, g, h, a, b, c, d, e, X);
			ROUND_16_63(i + 4, e, f, g, h, a, b, c, d, X);
			ROUND_16_63(i + 5, d, e, f, g, h, a, b, c, X);
			ROUND_16_63(i + 6, c, d, e, f, g, h, a, b, X);
			ROUND_16_63(i + 7, b, c, d, e, f, g, h, a, X);
		}

		ctx->h[0] += a;
		ctx->h[1] += b;
		ctx->h[2] += c;
		ctx->h[3] += d;
		ctx->h[4] += e;
		ctx->h[5] += f;
		ctx->h[6] += g;
		ctx->h[7] += h;

	}
}

void sha256_cleanse(void *ptr, size_t len)
{
	memset_func(ptr, 0, len);
}