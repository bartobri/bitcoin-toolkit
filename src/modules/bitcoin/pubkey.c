#include <stdio.h>
#include <gmp.h>
#include "pubkey.h"
#include "privkey.h"
#include "pubkey/point.h"

// TODO - proper error handling
PubKey pubkey_get(PrivKey p) {
	size_t i, j;
	mpz_t prvkey;
	Point pubkey;
	Point points[PRIVKEY_LENGTH * 8];
	PubKey r;

	mpz_init(prvkey);
	
	point_init(&pubkey);

	mpz_import(prvkey, PRIVKEY_LENGTH, 1, 1, 1, 0, p.data);
	
	for (i = 0; i < PRIVKEY_LENGTH * 8; ++i) {
		point_init(&(points[i]));
	}

	// Calculating public key
	point_set_generator(&points[0]);
	for (i = 1; i < PRIVKEY_LENGTH * 8; ++i) {

		point_double(&points[i], points[i-1]);

		if (!point_verify(points[i])) {
			printf("ERROR - Invalid point doubling!\n");
			return;
		}
	}

	// Add all points corresponding to 1 bits
	for (i = 0; i < PRIVKEY_LENGTH * 8; ++i) {
		if (mpz_tstbit(prvkey, i) == 1) {

			if (mpz_cmp_ui(pubkey.x, 0) == 0 && mpz_cmp_ui(pubkey.y, 0) == 0) {
				point_set(&pubkey, points[i]);
			} else {
				point_add(&pubkey, pubkey, points[i]);
			}

			if (!point_verify(pubkey)) {
				printf("ERROR - Invalid point addition!\n");
				return;
			}
		}
	}
	
	// Exporting x,y coordinates as byte string, making sure to write leading
	// zeros if either exports as less than 32 bytes.
	r.data[0] = 0x04;
	mpz_export(r.data + 1, &i, 1, 1, 1, 0, pubkey.x);
	while (i < 32) {
		for (j = 32; j >= 2; --j) {
			r.data[j] = r.data[j - 1];
		}
		r.data[1] = 0x00;
		++i;
	}
	mpz_export(r.data + 33, &i, 1, 1, 1, 0, pubkey.y);
	while (i < 32) {
		for (j = 64; j >= 34; --j) {
			r.data[j] = r.data[j - 1];
		}
		r.data[33] = 0x00;
		++i;
	}
	
	// Clear all points
	mpz_clear(prvkey);
	for (i = 0; i < PRIVKEY_LENGTH * 8; ++i) {
		point_clear(&(points[i]));
	}
	
	return r;
}
