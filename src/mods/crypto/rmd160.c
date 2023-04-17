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
 * Modified
 */

#include <stdio.h>
#include <string.h>
#include "rmd160.h"
#include "rmdconst.h"

#define DATA_ORDER_IS_LITTLE_ENDIAN

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

#define RIP1(a,b,c,d,e,w,s) { \
		a+=F1(b,c,d)+X(w); \
		a=ROTATE(a,s)+e; \
		c=ROTATE(c,10); }

#define RIP2(a,b,c,d,e,w,s,K) { \
		a+=F2(b,c,d)+X(w)+K; \
		a=ROTATE(a,s)+e; \
		c=ROTATE(c,10); }

#define RIP3(a,b,c,d,e,w,s,K) { \
		a+=F3(b,c,d)+X(w)+K; \
		a=ROTATE(a,s)+e; \
		c=ROTATE(c,10); }

#define RIP4(a,b,c,d,e,w,s,K) { \
		a+=F4(b,c,d)+X(w)+K; \
		a=ROTATE(a,s)+e; \
		c=ROTATE(c,10); }

#define RIP5(a,b,c,d,e,w,s,K) { \
		a+=F5(b,c,d)+X(w)+K; \
		a=ROTATE(a,s)+e; \
		c=ROTATE(c,10); }

#define HASH_MAKE_STRING(c,s)   do {    \
		unsigned long ll;               \
		ll=(c)->A; (void)HOST_l2c(ll,(s));      \
		ll=(c)->B; (void)HOST_l2c(ll,(s));      \
		ll=(c)->C; (void)HOST_l2c(ll,(s));      \
		ll=(c)->D; (void)HOST_l2c(ll,(s));      \
		ll=(c)->E; (void)HOST_l2c(ll,(s));      \
		} while (0)

#define F1(x,y,z)       ((x) ^ (y) ^ (z))
#define F2(x,y,z)       ((((y) ^ (z)) & (x)) ^ (z))
#define F3(x,y,z)       (((~(y)) | (x)) ^ (z))
#define F4(x,y,z)       ((((x) ^ (y)) & (z)) ^ (y))
#define F5(x,y,z)       (((~(z)) | (y)) ^ (x))

#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))

#define RIPEMD160_A     0x67452301L
#define RIPEMD160_B     0xEFCDAB89L
#define RIPEMD160_C     0x98BADCFEL
#define RIPEMD160_D     0x10325476L
#define RIPEMD160_E     0xC3D2E1F0L

void ripemd160_block_data_order(RIPEMD160_CTX *c, const void *p, size_t num);
void OPENSSL_cleanse(void *ptr, size_t len);

typedef void *(*memset_t)(void *, int, size_t);
static volatile memset_t memset_func = memset;

int RIPEMD160_Init(RIPEMD160_CTX *c)
{
	memset(c, 0, sizeof(*c));
	c->A = RIPEMD160_A;
	c->B = RIPEMD160_B;
	c->C = RIPEMD160_C;
	c->D = RIPEMD160_D;
	c->E = RIPEMD160_E;
	return 1;
}

int RIPEMD160_Update(RIPEMD160_CTX *c, const void *data_, size_t len)
{
	const unsigned char *data = data_;
	unsigned char *p;
	RIPEMD160_LONG l;
	size_t n;

	if (len == 0)
		return 1;

	l = (c->Nl + (((RIPEMD160_LONG) len) << 3)) & 0xffffffffUL;
	if (l < c->Nl)              /* overflow */
		c->Nh++;
	c->Nh += (RIPEMD160_LONG) (len >> 29); /* might cause compiler warning on
									   * 16-bit */
	c->Nl = l;

	n = c->num;
	if (n != 0) {
		p = (unsigned char *)c->data;

		if (len >= RIPEMD160_CBLOCK || len + n >= RIPEMD160_CBLOCK) {
			memcpy(p + n, data, RIPEMD160_CBLOCK - n);
			ripemd160_block_data_order(c, p, 1);
			n = RIPEMD160_CBLOCK - n;
			data += n;
			len -= n;
			c->num = 0;
			/*
			 * We use memset rather than OPENSSL_cleanse() here deliberately.
			 * Using OPENSSL_cleanse() here could be a performance issue. It
			 * will get properly cleansed on finalisation so this isn't a
			 * security problem.
			 */
			memset(p, 0, RIPEMD160_CBLOCK); /* keep it zeroed */
		} else {
			memcpy(p + n, data, len);
			c->num += (unsigned int)len;
			return 1;
		}
	}

	n = len / RIPEMD160_CBLOCK;
	if (n > 0) {
		ripemd160_block_data_order(c, data, n);
		n *= RIPEMD160_CBLOCK;
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

int RIPEMD160_Final(unsigned char *md, RIPEMD160_CTX *c)
{
	unsigned char *p = (unsigned char *)c->data;
	size_t n = c->num;

	p[n] = 0x80;                /* there is always room for one */
	n++;

	if (n > (RIPEMD160_CBLOCK - 8)) {
		memset(p + n, 0, RIPEMD160_CBLOCK - n);
		n = 0;
		ripemd160_block_data_order(c, p, 1);
	}
	memset(p + n, 0, RIPEMD160_CBLOCK - 8 - n);

	p += RIPEMD160_CBLOCK - 8;
#if defined(DATA_ORDER_IS_BIG_ENDIAN)
	(void)HOST_l2c(c->Nh, p);
	(void)HOST_l2c(c->Nl, p);
#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
	(void)HOST_l2c(c->Nl, p);
	(void)HOST_l2c(c->Nh, p);
#endif
	p -= RIPEMD160_CBLOCK;
	ripemd160_block_data_order(c, p, 1);
	c->num = 0;
	OPENSSL_cleanse(p, RIPEMD160_CBLOCK);

	HASH_MAKE_STRING(c, md);

	return 1;
}

void ripemd160_block_data_order(RIPEMD160_CTX *ctx, const void *p, size_t num)
{
	const unsigned char *data = p;
	register unsigned int A, B, C, D, E;
	unsigned int a, b, c, d, e, l;
	unsigned int XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
		XX8, XX9, XX10, XX11, XX12, XX13, XX14, XX15;
#define X(i)   XX##i

	for (; num--;) {

		A = ctx->A;
		B = ctx->B;
		C = ctx->C;
		D = ctx->D;
		E = ctx->E;

		(void)HOST_c2l(data, l);
		X(0) = l;
		(void)HOST_c2l(data, l);
		X(1) = l;
		RIP1(A, B, C, D, E, WL00, SL00);
		(void)HOST_c2l(data, l);
		X(2) = l;
		RIP1(E, A, B, C, D, WL01, SL01);
		(void)HOST_c2l(data, l);
		X(3) = l;
		RIP1(D, E, A, B, C, WL02, SL02);
		(void)HOST_c2l(data, l);
		X(4) = l;
		RIP1(C, D, E, A, B, WL03, SL03);
		(void)HOST_c2l(data, l);
		X(5) = l;
		RIP1(B, C, D, E, A, WL04, SL04);
		(void)HOST_c2l(data, l);
		X(6) = l;
		RIP1(A, B, C, D, E, WL05, SL05);
		(void)HOST_c2l(data, l);
		X(7) = l;
		RIP1(E, A, B, C, D, WL06, SL06);
		(void)HOST_c2l(data, l);
		X(8) = l;
		RIP1(D, E, A, B, C, WL07, SL07);
		(void)HOST_c2l(data, l);
		X(9) = l;
		RIP1(C, D, E, A, B, WL08, SL08);
		(void)HOST_c2l(data, l);
		X(10) = l;
		RIP1(B, C, D, E, A, WL09, SL09);
		(void)HOST_c2l(data, l);
		X(11) = l;
		RIP1(A, B, C, D, E, WL10, SL10);
		(void)HOST_c2l(data, l);
		X(12) = l;
		RIP1(E, A, B, C, D, WL11, SL11);
		(void)HOST_c2l(data, l);
		X(13) = l;
		RIP1(D, E, A, B, C, WL12, SL12);
		(void)HOST_c2l(data, l);
		X(14) = l;
		RIP1(C, D, E, A, B, WL13, SL13);
		(void)HOST_c2l(data, l);
		X(15) = l;
		RIP1(B, C, D, E, A, WL14, SL14);
		RIP1(A, B, C, D, E, WL15, SL15);

		RIP2(E, A, B, C, D, WL16, SL16, KL1);
		RIP2(D, E, A, B, C, WL17, SL17, KL1);
		RIP2(C, D, E, A, B, WL18, SL18, KL1);
		RIP2(B, C, D, E, A, WL19, SL19, KL1);
		RIP2(A, B, C, D, E, WL20, SL20, KL1);
		RIP2(E, A, B, C, D, WL21, SL21, KL1);
		RIP2(D, E, A, B, C, WL22, SL22, KL1);
		RIP2(C, D, E, A, B, WL23, SL23, KL1);
		RIP2(B, C, D, E, A, WL24, SL24, KL1);
		RIP2(A, B, C, D, E, WL25, SL25, KL1);
		RIP2(E, A, B, C, D, WL26, SL26, KL1);
		RIP2(D, E, A, B, C, WL27, SL27, KL1);
		RIP2(C, D, E, A, B, WL28, SL28, KL1);
		RIP2(B, C, D, E, A, WL29, SL29, KL1);
		RIP2(A, B, C, D, E, WL30, SL30, KL1);
		RIP2(E, A, B, C, D, WL31, SL31, KL1);

		RIP3(D, E, A, B, C, WL32, SL32, KL2);
		RIP3(C, D, E, A, B, WL33, SL33, KL2);
		RIP3(B, C, D, E, A, WL34, SL34, KL2);
		RIP3(A, B, C, D, E, WL35, SL35, KL2);
		RIP3(E, A, B, C, D, WL36, SL36, KL2);
		RIP3(D, E, A, B, C, WL37, SL37, KL2);
		RIP3(C, D, E, A, B, WL38, SL38, KL2);
		RIP3(B, C, D, E, A, WL39, SL39, KL2);
		RIP3(A, B, C, D, E, WL40, SL40, KL2);
		RIP3(E, A, B, C, D, WL41, SL41, KL2);
		RIP3(D, E, A, B, C, WL42, SL42, KL2);
		RIP3(C, D, E, A, B, WL43, SL43, KL2);
		RIP3(B, C, D, E, A, WL44, SL44, KL2);
		RIP3(A, B, C, D, E, WL45, SL45, KL2);
		RIP3(E, A, B, C, D, WL46, SL46, KL2);
		RIP3(D, E, A, B, C, WL47, SL47, KL2);

		RIP4(C, D, E, A, B, WL48, SL48, KL3);
		RIP4(B, C, D, E, A, WL49, SL49, KL3);
		RIP4(A, B, C, D, E, WL50, SL50, KL3);
		RIP4(E, A, B, C, D, WL51, SL51, KL3);
		RIP4(D, E, A, B, C, WL52, SL52, KL3);
		RIP4(C, D, E, A, B, WL53, SL53, KL3);
		RIP4(B, C, D, E, A, WL54, SL54, KL3);
		RIP4(A, B, C, D, E, WL55, SL55, KL3);
		RIP4(E, A, B, C, D, WL56, SL56, KL3);
		RIP4(D, E, A, B, C, WL57, SL57, KL3);
		RIP4(C, D, E, A, B, WL58, SL58, KL3);
		RIP4(B, C, D, E, A, WL59, SL59, KL3);
		RIP4(A, B, C, D, E, WL60, SL60, KL3);
		RIP4(E, A, B, C, D, WL61, SL61, KL3);
		RIP4(D, E, A, B, C, WL62, SL62, KL3);
		RIP4(C, D, E, A, B, WL63, SL63, KL3);

		RIP5(B, C, D, E, A, WL64, SL64, KL4);
		RIP5(A, B, C, D, E, WL65, SL65, KL4);
		RIP5(E, A, B, C, D, WL66, SL66, KL4);
		RIP5(D, E, A, B, C, WL67, SL67, KL4);
		RIP5(C, D, E, A, B, WL68, SL68, KL4);
		RIP5(B, C, D, E, A, WL69, SL69, KL4);
		RIP5(A, B, C, D, E, WL70, SL70, KL4);
		RIP5(E, A, B, C, D, WL71, SL71, KL4);
		RIP5(D, E, A, B, C, WL72, SL72, KL4);
		RIP5(C, D, E, A, B, WL73, SL73, KL4);
		RIP5(B, C, D, E, A, WL74, SL74, KL4);
		RIP5(A, B, C, D, E, WL75, SL75, KL4);
		RIP5(E, A, B, C, D, WL76, SL76, KL4);
		RIP5(D, E, A, B, C, WL77, SL77, KL4);
		RIP5(C, D, E, A, B, WL78, SL78, KL4);
		RIP5(B, C, D, E, A, WL79, SL79, KL4);

		a = A;
		b = B;
		c = C;
		d = D;
		e = E;
		/* Do other half */
		A = ctx->A;
		B = ctx->B;
		C = ctx->C;
		D = ctx->D;
		E = ctx->E;

		RIP5(A, B, C, D, E, WR00, SR00, KR0);
		RIP5(E, A, B, C, D, WR01, SR01, KR0);
		RIP5(D, E, A, B, C, WR02, SR02, KR0);
		RIP5(C, D, E, A, B, WR03, SR03, KR0);
		RIP5(B, C, D, E, A, WR04, SR04, KR0);
		RIP5(A, B, C, D, E, WR05, SR05, KR0);
		RIP5(E, A, B, C, D, WR06, SR06, KR0);
		RIP5(D, E, A, B, C, WR07, SR07, KR0);
		RIP5(C, D, E, A, B, WR08, SR08, KR0);
		RIP5(B, C, D, E, A, WR09, SR09, KR0);
		RIP5(A, B, C, D, E, WR10, SR10, KR0);
		RIP5(E, A, B, C, D, WR11, SR11, KR0);
		RIP5(D, E, A, B, C, WR12, SR12, KR0);
		RIP5(C, D, E, A, B, WR13, SR13, KR0);
		RIP5(B, C, D, E, A, WR14, SR14, KR0);
		RIP5(A, B, C, D, E, WR15, SR15, KR0);

		RIP4(E, A, B, C, D, WR16, SR16, KR1);
		RIP4(D, E, A, B, C, WR17, SR17, KR1);
		RIP4(C, D, E, A, B, WR18, SR18, KR1);
		RIP4(B, C, D, E, A, WR19, SR19, KR1);
		RIP4(A, B, C, D, E, WR20, SR20, KR1);
		RIP4(E, A, B, C, D, WR21, SR21, KR1);
		RIP4(D, E, A, B, C, WR22, SR22, KR1);
		RIP4(C, D, E, A, B, WR23, SR23, KR1);
		RIP4(B, C, D, E, A, WR24, SR24, KR1);
		RIP4(A, B, C, D, E, WR25, SR25, KR1);
		RIP4(E, A, B, C, D, WR26, SR26, KR1);
		RIP4(D, E, A, B, C, WR27, SR27, KR1);
		RIP4(C, D, E, A, B, WR28, SR28, KR1);
		RIP4(B, C, D, E, A, WR29, SR29, KR1);
		RIP4(A, B, C, D, E, WR30, SR30, KR1);
		RIP4(E, A, B, C, D, WR31, SR31, KR1);

		RIP3(D, E, A, B, C, WR32, SR32, KR2);
		RIP3(C, D, E, A, B, WR33, SR33, KR2);
		RIP3(B, C, D, E, A, WR34, SR34, KR2);
		RIP3(A, B, C, D, E, WR35, SR35, KR2);
		RIP3(E, A, B, C, D, WR36, SR36, KR2);
		RIP3(D, E, A, B, C, WR37, SR37, KR2);
		RIP3(C, D, E, A, B, WR38, SR38, KR2);
		RIP3(B, C, D, E, A, WR39, SR39, KR2);
		RIP3(A, B, C, D, E, WR40, SR40, KR2);
		RIP3(E, A, B, C, D, WR41, SR41, KR2);
		RIP3(D, E, A, B, C, WR42, SR42, KR2);
		RIP3(C, D, E, A, B, WR43, SR43, KR2);
		RIP3(B, C, D, E, A, WR44, SR44, KR2);
		RIP3(A, B, C, D, E, WR45, SR45, KR2);
		RIP3(E, A, B, C, D, WR46, SR46, KR2);
		RIP3(D, E, A, B, C, WR47, SR47, KR2);

		RIP2(C, D, E, A, B, WR48, SR48, KR3);
		RIP2(B, C, D, E, A, WR49, SR49, KR3);
		RIP2(A, B, C, D, E, WR50, SR50, KR3);
		RIP2(E, A, B, C, D, WR51, SR51, KR3);
		RIP2(D, E, A, B, C, WR52, SR52, KR3);
		RIP2(C, D, E, A, B, WR53, SR53, KR3);
		RIP2(B, C, D, E, A, WR54, SR54, KR3);
		RIP2(A, B, C, D, E, WR55, SR55, KR3);
		RIP2(E, A, B, C, D, WR56, SR56, KR3);
		RIP2(D, E, A, B, C, WR57, SR57, KR3);
		RIP2(C, D, E, A, B, WR58, SR58, KR3);
		RIP2(B, C, D, E, A, WR59, SR59, KR3);
		RIP2(A, B, C, D, E, WR60, SR60, KR3);
		RIP2(E, A, B, C, D, WR61, SR61, KR3);
		RIP2(D, E, A, B, C, WR62, SR62, KR3);
		RIP2(C, D, E, A, B, WR63, SR63, KR3);

		RIP1(B, C, D, E, A, WR64, SR64);
		RIP1(A, B, C, D, E, WR65, SR65);
		RIP1(E, A, B, C, D, WR66, SR66);
		RIP1(D, E, A, B, C, WR67, SR67);
		RIP1(C, D, E, A, B, WR68, SR68);
		RIP1(B, C, D, E, A, WR69, SR69);
		RIP1(A, B, C, D, E, WR70, SR70);
		RIP1(E, A, B, C, D, WR71, SR71);
		RIP1(D, E, A, B, C, WR72, SR72);
		RIP1(C, D, E, A, B, WR73, SR73);
		RIP1(B, C, D, E, A, WR74, SR74);
		RIP1(A, B, C, D, E, WR75, SR75);
		RIP1(E, A, B, C, D, WR76, SR76);
		RIP1(D, E, A, B, C, WR77, SR77);
		RIP1(C, D, E, A, B, WR78, SR78);
		RIP1(B, C, D, E, A, WR79, SR79);

		D = ctx->B + c + D;
		ctx->B = ctx->C + d + E;
		ctx->C = ctx->D + e + A;
		ctx->D = ctx->E + a + B;
		ctx->E = ctx->A + b + C;
		ctx->A = D;

	}
}

void OPENSSL_cleanse(void *ptr, size_t len)
{
	memset_func(ptr, 0, len);
}