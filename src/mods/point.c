#include <gmp.h>
#include "point.h"
#include "assert.h"

#define BITCOIN_PRIME "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"
#define BITCOIN_GENERATOR_POINT_X "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798"
#define BITCOIN_GENERATOR_POINT_Y "483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8"

void point_math_inversemod(mpz_t, mpz_t, mpz_t);

void point_init(Point *p) {
	assert(p);
	mpz_init(p->x);
	mpz_init(p->y);
}

void point_set(Point *a, Point b) {
	assert(a);
	assert(b.x && b.y);
	mpz_set(a->x, b.x);
	mpz_set(a->y, b.y);
}

void point_set_generator(Point *a) {
	mpz_t gx, gy;
	
	assert(a);
	
	mpz_init(gx);
	mpz_init(gy);
	
	mpz_set_str(gx, BITCOIN_GENERATOR_POINT_X, 16);
	mpz_set_str(gy, BITCOIN_GENERATOR_POINT_Y, 16);
	
	mpz_set(a->x, gx);
	mpz_set(a->y, gy);
	
	mpz_clear(gx);
	mpz_clear(gy);
}

void point_double(Point *result, Point a) {
	mpz_t tempx, tempy, p, slope;
	
	assert(result);
	assert(a.x && a.y);
	
	mpz_init(tempx);
	mpz_init(tempy);
	mpz_init(p);
	mpz_init(slope);
	
	mpz_set_str(p, BITCOIN_PRIME, 16);
	
	// slope = ((3 * x^2) * inverseMod((2*y), p)) % p
	mpz_pow_ui(tempx, a.x, 2);
	mpz_mul_ui(tempx, tempx, 3);
	mpz_mul_ui(tempy, a.y, 2);
	point_math_inversemod(tempy, tempy, p);
	mpz_mul(slope, tempx, tempy);
	mpz_mod(slope, slope, p);

	// xdbl = slope^2 - 2*x
	mpz_mul_ui(tempx, a.x, 2);
	mpz_pow_ui(result->x, slope, 2);
	mpz_sub(result->x, result->x, tempx);
	mpz_mod(result->x, result->x, p);

	// ydbl = slope *(x-xdbl)-y
	mpz_sub(tempx, a.x, result->x);
	mpz_mul(tempx, slope, tempx);
	mpz_sub(result->y, tempx, a.y);
	mpz_mod(result->y, result->y, p);
	
	mpz_clear(tempx);
	mpz_clear(tempy);
	mpz_clear(p);
	mpz_clear(slope);
}

void point_add(Point *result, Point a, Point b) {
	mpz_t tempx, tempy, sumx, sumy, p, slope;
	
	assert(result);
	assert(a.x && a.y);
	assert(b.x && b.y);
	
	mpz_init(tempx);
	mpz_init(tempy);
	mpz_init(sumx);
	mpz_init(sumy);
	mpz_init(p);
	mpz_init(slope);
	
	mpz_set_str(p, BITCOIN_PRIME, 16);
	
	// slope = (y1-y2) * inverseMod(x1-x2, p)
	mpz_sub(tempx, a.x, b.x);
	mpz_sub(tempy, a.y, b.y);
	point_math_inversemod(tempx, tempx, p);
	mpz_mul(slope, tempy, tempx);
	mpz_mod(slope, slope, p);
	
	// xsum = slope^2 - (x1+x2)
	mpz_pow_ui(tempy, slope, 2);
	mpz_add(tempx, a.x, b.x);
	mpz_sub(sumx, tempy, tempx);
	mpz_mod(sumx, sumx, p);
	
	// ysum = slope*(x1-xsum)-y1
	mpz_sub(tempx, a.x, sumx);
	mpz_mul(tempx, slope, tempx);
	mpz_sub(sumy, tempx, a.y);
	mpz_mod(sumy, sumy, p);
	
	mpz_set(result->x, sumx);
	mpz_set(result->y, sumy);
	
	mpz_clear(tempx);
	mpz_clear(tempy);
	mpz_clear(sumx);
	mpz_clear(sumy);
	mpz_clear(p);
	mpz_clear(slope);
}

void point_math_inversemod(mpz_t r, mpz_t a, mpz_t m) {
	mpz_t c, d, uc, vc, ud, vd, rem, q, uct, vct, udt, vdt, temp;
	
	mpz_init(c);
	mpz_init(d);
	mpz_init(uc);
	mpz_init(vc);
	mpz_init(ud);
	mpz_init(vd);
	mpz_init(rem);
	mpz_init(q);
	mpz_init(uct);
	mpz_init(vct);
	mpz_init(udt);
	mpz_init(vdt);
	mpz_init(temp);
	
	while (mpz_cmp_ui(a, 0) < 0) {
		mpz_add(a, a, m);
	}
	
	if (mpz_cmp_ui(a, 0) < 0 || mpz_cmp(m, a) <= 0) {
		mpz_mod(a, a, m);
	}
	
	mpz_set(c, a);
	mpz_set(d, m);
	mpz_set_ui(uc, 1);
	mpz_set_ui(vc, 0);
	mpz_set_ui(ud, 0);
	mpz_set_ui(vd, 1);
	
	while (mpz_cmp_ui(c, 0) != 0) {
		mpz_tdiv_q(q, d, c);
		mpz_mod(rem, d, c);
		
		mpz_set(d, c);
		mpz_set(c, rem);
		
		mpz_set(uct, uc);
		mpz_set(vct, vc);
		mpz_set(udt, ud);
		mpz_set(vdt, vd);
		
		mpz_mul(temp, q, uct);
		mpz_sub(uc, udt, temp);
		mpz_mul(temp, q, vct);
		mpz_sub(vc, vdt, temp);
		mpz_set(ud, uct);
		mpz_set(vd, vct);
	}
	
	if (mpz_cmp_ui(ud, 0) > 0) {
		mpz_set(r, ud);
	} else {
		mpz_add(r, ud, m);
	}
	
	mpz_clear(c);
	mpz_clear(d);
	mpz_clear(uc);
	mpz_clear(vc);
	mpz_clear(ud);
	mpz_clear(vd);
	mpz_clear(rem);
	mpz_clear(q);
	mpz_clear(uct);
	mpz_clear(vct);
	mpz_clear(udt);
	mpz_clear(vdt);
	mpz_clear(temp);
}

int point_verify(Point a) {
	int r = 0;
	mpz_t tempx, tempy, tempr, p;
	
	assert(a.x && a.y);
	
	mpz_init(tempx);
	mpz_init(tempy);
	mpz_init(tempr);
	mpz_init(p);
	
	mpz_set_str(p, BITCOIN_PRIME, 16);
	
	mpz_set(tempx, a.x);
	mpz_set(tempy, a.y);
	
	// ((x^3) + 7 - (y^2)) % p == 0
	mpz_pow_ui(tempx, tempx, 3);
	mpz_pow_ui(tempy, tempy, 2);
	mpz_add_ui(tempx, tempx, 7);
	mpz_sub(tempr, tempx, tempy);
	mpz_mod(tempr, tempr, p);
	
	if (mpz_cmp_ui(tempr, 0) == 0) {
		r = 1;
	}
	
	mpz_clear(tempx);
	mpz_clear(tempy);
	mpz_clear(tempr);
	mpz_clear(p);
	
	return r;
}

void point_clear(Point *p) {
	mpz_clear(p->x);
	mpz_clear(p->y);
}
