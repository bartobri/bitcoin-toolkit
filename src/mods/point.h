/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef POINT_H
#define POINT_H 1

#include <gmp.h>
#ifdef GMP_H_MISSING
#   include "GMP/mini-gmp.h"
#endif

typedef struct Point *Point;
struct Point
{
	mpz_t x;
	mpz_t y;
};

void point_init(Point);
void point_set(Point, Point);
void point_set_generator(Point);
void point_double(Point, Point);
void point_add(Point, Point, Point);
void point_solve_y(Point, unsigned char);
int  point_verify(Point);
void point_clear(Point);

#endif
