#include <stdio.h>
#include <gcrypt.h>

#include "address.h"
#include "point.h"
#include "random.h"
#include "base58.h"

// (1.158 * 10^77) - 1
#define PRIVATE_KEY_MAX "100047a327efc14f7fe934ae56989375080f11619ff7157ffffffffffffffffff"

//void address_get(KeyPair *key) {
void address_get(unsigned char *prv, unsigned char *pub) {
	size_t i, j;
	mpz_t prvkey;
	Point pubkey;
	Point points[PRIVATE_KEY_BYTES * 8];

	mpz_init(prvkey);
	
	point_init(&pubkey);

	mpz_import(prvkey, PRIVATE_KEY_BYTES, 1, 1, 1, 0, prv);
	
	for (i = 0; i < PRIVATE_KEY_BYTES * 8; ++i) {
		point_init(&(points[i]));
	}

	// Calculating public key
	point_set_generator(&points[0]);
	for (i = 1; i < PRIVATE_KEY_BYTES * 8; ++i) {

		point_double(&points[i], points[i-1]);

		if (!point_verify(points[i])) {
			printf("ERROR - Invalid point doubling!\n");
			return;
		}
	}

	// Add all points corresponding to 1 bits
	for (i = 0; i < PRIVATE_KEY_BYTES * 8; ++i) {
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
	pub[0] = 0x04;
	mpz_export(pub + 1, &i, 1, 1, 1, 0, pubkey.x);
	//if (i < 32) {
	//	fprintf(stderr, "X ZEROS WARNING!\n");
	//}
	while (i < 32) {
		for (j = 32; j >= 2; --j) {
			pub[j] = pub[j - 1];
		}
		pub[1] = 0x00;
		++i;
	}
	mpz_export(pub + 33, &i, 1, 1, 1, 0, pubkey.y);
	//if (i < 32) {
	//	fprintf(stderr, "Y ZEROS WARNING!\n");
	//}
	while (i < 32) {
		for (j = 64; j >= 34; --j) {
			pub[j] = pub[j - 1];
		}
		pub[33] = 0x00;
		++i;
	}
	
	mpz_clear(prvkey);
	for (i = 0; i < PRIVATE_KEY_BYTES * 8; ++i) {
		point_clear(&(points[i]));
	}
}

void address_print(unsigned char *pub) {
	size_t i;
	gcry_md_hd_t h1, h2, h3, h4;
	char *address;
	unsigned char checksum[4];
	unsigned char *sha256;
	unsigned char *rmd160;
	unsigned char *rmd160v;
	unsigned char *rmd160vc;

	// Initialize gcrypt libs
	if (!gcry_check_version(GCRYPT_VERSION)) {
		fputs ("libgcrypt version mismatch\n", stderr);
		return;
	}
	
	gcry_md_open(&h1, GCRY_MD_SHA256, 0);
	gcry_md_open(&h2, GCRY_MD_RMD160, 0);
	gcry_md_open(&h3, GCRY_MD_SHA256, 0);
	gcry_md_open(&h4, GCRY_MD_SHA256, 0);
	
	/*
	 * Public key -> SHA256
	 */
	for (i = 0; i < PUBLIC_KEY_BYTES; ++i) {
		gcry_md_putc(h1, pub[i]);
	}
	sha256 = gcry_md_read(h1, 0);
	
	/*
	 * SHA256 -> RMD160
	 */
	for (i = 0; i < 32; ++i) {
		gcry_md_putc(h2, sha256[i]);
	}
	rmd160 = gcry_md_read(h2, 0);
	
	/*
	 * Adding version bit to front
	 */
	rmd160v = malloc(21);
	rmd160v[0] = 0;
	for (i = 0; i < 20; ++i) {
		rmd160v[i+1] = rmd160[i];
	}
	
	/*
	 * Getting checksum of rmd160v (first 4 bytes of sha256(sha256(rmd160v)))
	 */
	for (i = 0; i < 21; ++i) {
		gcry_md_putc(h3, rmd160v[i]);
	}
	sha256 = gcry_md_read(h3, 0);
	for (i = 0; i < 32; ++i) {
		gcry_md_putc(h4, sha256[i]);
	}
	sha256 = gcry_md_read(h4, 0);
	checksum[0] = sha256[0];
	checksum[1] = sha256[1];
	checksum[2] = sha256[2];
	checksum[3] = sha256[3];
	
	/*
	 * Adding checksum to rmd160v
	 */
	rmd160vc = malloc(25);
	for (i = 0; i < 21; ++i) {
		rmd160vc[i] = rmd160v[i];
	}
	for (i = 0; i < 4; ++i) {
		rmd160vc[i+21] = checksum[i];
	}

	/*
	 * Encode rmd160vc in base58 and store in key structure
	 */
	address = base58_encode(rmd160vc, 25);
	
	printf("%s", address);
	
	free(address);
	
	gcry_md_close(h1);
	gcry_md_close(h2);
	gcry_md_close(h3);
	gcry_md_close(h4);
}
