#ifndef POINT_H
#define POINT_H 1

#include <gmp.h>

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
int  point_verify(Point);
void point_clear(Point);

#endif
