#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "pubkey.h"
#include "privkey.h"
#include "point.h"

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
			// TODO - handle error
			//printf("ERROR - Invalid point doubling!\n");
			//return;
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
				// TODO - handle error
				//printf("ERROR - Invalid point addition!\n");
				//return;
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

PubKeyComp pubkey_compress(PubKey k) {
	mpz_t x, y;
	size_t point_length = 32;
	PubKeyComp p;
	
	mpz_init(x);
	mpz_init(y);
	
	mpz_import(x, point_length, 1, 1, 1, 0, k.data + 1);
	mpz_import(y, point_length, 1, 1, 1, 0, k.data + 33);
	
	if (mpz_even_p(y)) {
		p.data[0] = 0x02;
	} else {
		p.data[0] = 0x03;
	}
	
	mpz_export(p.data + 1, &point_length, 1, 1, 1, 0, x);
	
	mpz_clear(x);
	mpz_clear(y);
	
	return p;
}

PubKeyComp pubkey_get_compressed(PrivKeyComp k) {
	int i;
	PrivKey t;
	PubKeyComp p;
	
	for (i = 0; i < PRIVKEY_COMP_LENGTH - 1; ++i) {
		t.data[i] = k.data[i];
	}
	
	p = pubkey_compress(pubkey_get(t));
	
	return p;
}

char *pubkey_to_hex(PubKey k) {
	int i;
	char *r;
	
	r = malloc((PUBKEY_LENGTH * 2) + 1);
	
	if (r == NULL) {
		return r;
	} else {
		memset(r, 0, sizeof(*r));
	}
	
	for (i = 0; i < PUBKEY_LENGTH; ++i) {
		sprintf(r + (i * 2), "%02x", k.data[i]);
	}
	
	return r;
}
