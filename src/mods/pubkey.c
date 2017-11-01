#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "pubkey.h"
#include "privkey.h"
#include "point.h"

#define PUBKEY_COMPRESSED_FLAG_EVEN   0x02
#define PUBKEY_COMPRESSED_FLAG_ODD    0x03
#define PUBKEY_UNCOMPRESSED_FLAG      0x04

// TODO - proper error handling
PubKey pubkey_get(PrivKey k) {
	size_t i, j;
	mpz_t prvkey;
	Point pubkey;
	Point points[PRIVKEY_LENGTH * 8];
	PubKey r;

	mpz_init(prvkey);
	
	point_init(&pubkey);

	mpz_import(prvkey, PRIVKEY_LENGTH, 1, 1, 1, 0, k->data);
	
	for (i = 0; i < PRIVKEY_LENGTH * 8; ++i) {
		point_init(points + i);
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
	
	// Setting compression flag
	if (privkey_is_compressed(k)) {
		if (mpz_even_p(pubkey.y)) {
			r.data[0] = PUBKEY_COMPRESSED_FLAG_EVEN;
		} else {
			r.data[0] = PUBKEY_COMPRESSED_FLAG_ODD;
		}
	} else {
		r.data[0] = PUBKEY_UNCOMPRESSED_FLAG;
	}

	// Exporting x,y coordinates as byte string, making sure to write leading
	// zeros if either exports as less than 32 bytes.
	mpz_export(r.data + 1, &i, 1, 1, 1, 0, pubkey.x);
	while (i < 32) {
		for (j = 32; j >= 2; --j) {
			r.data[j] = r.data[j - 1];
		}
		r.data[1] = 0x00;
		++i;
	}
	if (!privkey_is_compressed(k)) {
		mpz_export(r.data + 33, &i, 1, 1, 1, 0, pubkey.y);
		while (i < 32) {
			for (j = 64; j >= 34; --j) {
				r.data[j] = r.data[j - 1];
			}
			r.data[33] = 0x00;
			++i;
		}
	}

	// Clear all points
	mpz_clear(prvkey);
	for (i = 0; i < PRIVKEY_LENGTH * 8; ++i) {
		point_clear(points + i);
	}
	
	return r;
}

PubKey pubkey_compress(PubKey k) {
	mpz_t y;
	size_t point_length = 32;
	
	if (k.data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k.data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		return k;
	}

	mpz_init(y);

	mpz_import(y, point_length, 1, 1, 1, 0, k.data + 1 + point_length);
	
	if (mpz_even_p(y)) {
		k.data[0] = 0x02;
	} else {
		k.data[0] = 0x03;
	}

	mpz_clear(y);
	
	return k;
}

int pubkey_is_compressed(PubKey k) {
	if (k.data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k.data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		return 1;
	}
	
	return 0;
}

char *pubkey_to_hex(PubKey k) {
	int i, l;
	char *r;
	
	if (k.data[0] == PUBKEY_UNCOMPRESSED_FLAG) {
		l = (PUBKEY_UNCOMPRESSED_LENGTH * 2) + 2;
	} else if (k.data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k.data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		l = (PUBKEY_COMPRESSED_LENGTH * 2) + 2;
	} else {
		return NULL;
	}

	r = malloc(l + 1);
	if (r == NULL) {
		return NULL;
	}

	memset(r, 0, l + 1);
	
	for (i = 0; i < l/2; ++i) {
		sprintf(r + (i * 2), "%02x", k.data[i]);
	}
	
	return r;
}

