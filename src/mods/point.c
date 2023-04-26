/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <assert.h>
#include <gmp.h>
#ifdef GMP_H_MISSING
#   include "GMP/mini-gmp.h"
#endif
#include "point.h"

#define BITCOIN_PRIME             "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"
#define BITCOIN_GENERATOR_POINT_X "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798"
#define BITCOIN_GENERATOR_POINT_Y "483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8"

void point_init(Point p)
{
	assert(p);

	mpz_init(p->x);
	mpz_init(p->y);
}

void point_set(Point a, Point b)
{
	assert(a);
	assert(b);
	assert(b->x && b->y);

	mpz_set(a->x, b->x);
	mpz_set(a->y, b->y);
}

void point_set_generator(Point p)
{
	assert(p);
	
	mpz_set_str(p->x, BITCOIN_GENERATOR_POINT_X, 16);
	mpz_set_str(p->y, BITCOIN_GENERATOR_POINT_Y, 16);
}

void point_double(Point result, Point a)
{
	static mpz_t tempx, tempy, p, slope;
	static int init = 0;
	
	assert(result);
	assert(a->x && a->y);
	
	if (!init)
	{
		mpz_init(tempx);
		mpz_init(tempy);
		mpz_init(p);
		mpz_init(slope);
		init = 1;
	}
	
	mpz_set_str(p, BITCOIN_PRIME, 16);
	
	// slope = ((3 * x^2) * inverseMod((2*y), p)) % p
	mpz_pow_ui(tempx, a->x, 2);
	mpz_mul_ui(tempx, tempx, 3);
	mpz_mul_ui(tempy, a->y, 2);
	mpz_invert(tempy, tempy, p);
	mpz_mul(slope, tempx, tempy);
	mpz_mod(slope, slope, p);

	// xdbl = slope^2 - 2*x
	mpz_mul_ui(tempx, a->x, 2);
	mpz_pow_ui(result->x, slope, 2);
	mpz_sub(result->x, result->x, tempx);
	mpz_mod(result->x, result->x, p);

	// ydbl = slope *(x-xdbl)-y
	mpz_sub(tempx, a->x, result->x);
	mpz_mul(tempx, slope, tempx);
	mpz_sub(result->y, tempx, a->y);
	mpz_mod(result->y, result->y, p);
}

void point_add(Point result, Point a, Point b)
{
	static mpz_t tempx, tempy, sumx, sumy, p, slope;
	static int init = 0;
	
	assert(result);
	assert(a);
	assert(b);
	assert(a->x && a->y);
	assert(b->x && b->y);
	
	if (!init)
	{
		mpz_init(tempx);
		mpz_init(tempy);
		mpz_init(sumx);
		mpz_init(sumy);
		mpz_init(p);
		mpz_init(slope);
		init = 1;
	}
	
	mpz_set_str(p, BITCOIN_PRIME, 16);
	
	// slope = (y1-y2) * inverseMod(x1-x2, p)
	mpz_sub(tempx, a->x, b->x);
	mpz_sub(tempy, a->y, b->y);
	mpz_invert(tempx, tempx, p);
	mpz_mul(slope, tempy, tempx);
	mpz_mod(slope, slope, p);
	
	// xsum = slope^2 - (x1+x2)
	mpz_pow_ui(tempy, slope, 2);
	mpz_add(tempx, a->x, b->x);
	mpz_sub(sumx, tempy, tempx);
	mpz_mod(sumx, sumx, p);
	
	// ysum = slope*(x1-xsum)-y1
	mpz_sub(tempx, a->x, sumx);
	mpz_mul(tempx, slope, tempx);
	mpz_sub(sumy, tempx, a->y);
	mpz_mod(sumy, sumy, p);
	
	mpz_set(result->x, sumx);
	mpz_set(result->y, sumy);
}

void point_solve_y(Point point, unsigned char even_odd_flag)
{
	mpz_t tempx, tempy, exp, p;

	assert(point);
	assert(point->x);
	assert(point->y);

	mpz_init(tempx);
	mpz_init(tempy);
	mpz_init(exp);
	mpz_init(p);
	
	mpz_set_str(p, BITCOIN_PRIME, 16);

	// This calculates y squared: ((x^3 % p) + 7) % p
	mpz_powm_ui(tempx, point->x, 3, p);
	mpz_add_ui(tempx, tempx, 7);
	mpz_mod(tempx, tempx, p);

	// Next we calculate the exponent: ((p+1)/4)
	mpz_add_ui(exp, p, 1);
	mpz_tdiv_q_ui(exp, exp, 4);

	// Raise y squared to the power of exp (keeping modulo p) to get the square root
	mpz_powm(tempy, tempx, exp, p);

	// Determine odd or even
	if ((mpz_even_p(tempy) && (even_odd_flag & 1)) || (mpz_odd_p(tempy) && !(even_odd_flag & 1)))
	{
		mpz_neg(tempy, tempy);
		mpz_mod(tempy, tempy, p);
	}

	// set out y value
	mpz_set(point->y, tempy);

	mpz_clear(tempx);
	mpz_clear(tempy);
	mpz_clear(exp);
	mpz_clear(p);
}

int point_verify(Point a)
{
	int r = 0;
	static mpz_t tempx, tempy, tempr, p;
	static int init = 0;
	
	assert(a->x && a->y);
	
	if (!init)
	{
		mpz_init(tempx);
		mpz_init(tempy);
		mpz_init(tempr);
		mpz_init(p);
		init = 1;
	}
	
	mpz_set_str(p, BITCOIN_PRIME, 16);
	
	mpz_set(tempx, a->x);
	mpz_set(tempy, a->y);
	
	// ((x^3) + 7 - (y^2)) % p == 0
	mpz_pow_ui(tempx, tempx, 3);
	mpz_pow_ui(tempy, tempy, 2);
	mpz_add_ui(tempx, tempx, 7);
	mpz_sub(tempr, tempx, tempy);
	mpz_mod(tempr, tempr, p);
	
	if (mpz_cmp_ui(tempr, 0) == 0)
	{
		r = 1;
	}
	
	return r;
}

void point_clear(Point p)
{
	mpz_clear(p->x);
	mpz_clear(p->y);
}