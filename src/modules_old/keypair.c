#include <stdio.h>
#include <gcrypt.h>

#include "keypair.h"
#include "point.h"
#include "random.h"
#include "base58.h"

// (1.158 * 10^77) - 1
#define PRIVATE_KEY_MAX "100047a327efc14f7fe934ae56989375080f11619ff7157ffffffffffffffffff"

void keypair_create_private(KeyPair *);
void keypair_calculate_public(KeyPair *);
void keypair_compress_public(KeyPair *);
void keypair_compress_private(KeyPair *);

KeyPair keypair_new(void) {
	KeyPair key;
	
	// Create new private key
	keypair_create_private(&key);
	
	// Calculate public key from private key
	keypair_compress_private(&key);
	
	// Calculate public key from private key
	keypair_calculate_public(&key);
	
	// Calculate public key from private key
	keypair_compress_public(&key);
	
	return key;
}

void keypair_dump(KeyPair key) {
	int i;

	printf("Private Key (hex): ");
	for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
		printf("%02x", key.private_key[i]);
	}
	printf("\n");
	
	printf("Private Key Compressed (hex): ");
	for (i = 0; i < PRIVATE_KEY_COMPRESSED_BYTES; ++i) {
		printf("%02x", key.private_key_compressed[i]);
	}
	printf("\n");
	
	printf("Public Key (hex): ");
	for (i = 0; i < PUBLIC_KEY_BYTES; ++i) {
		printf("%02x", key.public_key[i]);
	}
	printf("\n");
	
	printf("Public Key Compressed (hex): ");
	for (i = 0; i < PUBLIC_KEY_COMPRESSED_BYTES; ++i) {
		printf("%02x", key.public_key_compressed[i]);
	}
	printf("\n");
}

void keypair_create_private(KeyPair *key) {
	int i;
	mpz_t cur_key, max_key;

	// Init and set max key size
	mpz_init(max_key);
	mpz_init(cur_key);
	mpz_set_str(max_key, PRIVATE_KEY_MAX, 16);
	
	// Creating private key as hex string
	while (mpz_cmp_ui(cur_key, 1) <= 0 || mpz_cmp(cur_key, max_key) >= 0) {

		for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
			key->private_key[i] = random_get_byte();
		}
		mpz_import(cur_key, PRIVATE_KEY_BYTES, 1, 1, 1, 0, key->private_key);
	}
	
	// FOR DEBUGGING - Remove me
	//mpz_set_str(key->priv, "18E14A7B6A307F426A94F8114701E7C8E774E7F9A47E2C2035DB29A206321725", 16);
	
	mpz_clear(max_key);
	mpz_clear(cur_key);
}

void keypair_compress_private(KeyPair *key) {
	int i;
	
	for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
		key->private_key_compressed[i] = key->private_key[i];
	}
	key->private_key_compressed[i] = 0x01;
}

void keypair_calculate_public(KeyPair *key) {
	size_t i, j;
	mpz_t prvkey;
	Point pubkey;
	Point points[PRIVATE_KEY_BYTES * 8];

	mpz_init(prvkey);
	
	point_init(&pubkey);

	mpz_import(prvkey, PRIVATE_KEY_BYTES, 1, 1, 1, 0, key->private_key);
	
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
	key->public_key[0] = 0x04;
	mpz_export(key->public_key + 1, &i, 1, 1, 1, 0, pubkey.x);
	while (i < 32) {
		for (j = 32; j >= 2; --j) {
			key->public_key[j] = key->public_key[j - 1];
		}
		key->public_key[1] = 0x00;
		++i;
	}
	mpz_export(key->public_key + 33, &i, 1, 1, 1, 0, pubkey.y);
	while (i < 32) {
		for (j = 64; j >= 34; --j) {
			key->public_key[j] = key->public_key[j - 1];
		}
		key->public_key[33] = 0x00;
		++i;
	}
	
	mpz_clear(prvkey);
	for (i = 0; i < PRIVATE_KEY_BYTES * 8; ++i) {
		point_clear(&(points[i]));
	}
}

void keypair_compress_public(KeyPair *key) {
	mpz_t x, y;
	size_t point_length = 32;
	
	mpz_init(x);
	mpz_init(y);
	
	mpz_import(x, point_length, 1, 1, 1, 0, key->public_key + 1);
	mpz_import(y, point_length, 1, 1, 1, 0, key->public_key + 33);
	
	if (mpz_even_p(y)) {
		key->public_key_compressed[0] = 0x02;
	} else {
		key->public_key_compressed[0] = 0x03;
	}
	
	mpz_export(key->public_key_compressed + 1, &point_length, 1, 1, 1, 0, x);
	
	mpz_clear(x);
	mpz_clear(y);
}

void keypair_print_address(KeyPair *key) {
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
		gcry_md_putc(h1, key->public_key[i]);
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

void keypair_print_wif(KeyPair *key) {
	int i;
	char *wif;
	gcry_md_hd_t h1, h2;
	unsigned char *sha256;
	unsigned char gar[37];  // 32 (sha256) plus 4 byte checksum and 1 byte prefix
	
	// Initialize gcrypt libs
	if (!gcry_check_version(GCRYPT_VERSION)) {
		fputs ("libgcrypt version mismatch\n", stderr);
		return;
	}
	
	gcry_md_open(&h1, GCRY_MD_SHA256, 0);
	gcry_md_open(&h2, GCRY_MD_SHA256, 0);
	
	// 0x80 prefix for mainnet, 0xEF for testnet
	gar[0] = 0x80;
	for (i = 0; i < PRIVATE_KEY_BYTES; ++i) {
		gar[i+1] = key->private_key[i];
	}
	
	/*
	 * Private key -> SHA256
	 */
	for (i = 0; i < PRIVATE_KEY_BYTES+1; ++i) {
		gcry_md_putc(h1, gar[i]);
	}
	sha256 = gcry_md_read(h1, 0);
	
	/*
	 * SHA256 -> SHA256
	 */
	for (i = 0; i < 32; ++i) {
		gcry_md_putc(h2, sha256[i]);
	}
	sha256 = gcry_md_read(h2, 0);
	
	/*
	 * Get checksum (first 4 bytes)
	 */
	gar[33] = sha256[0];
	gar[34] = sha256[1];
	gar[35] = sha256[2];
	gar[36] = sha256[3];
	
	wif = base58_encode(gar, 37);
	
	printf("%s", wif);
	
	gcry_md_close(h1);
	gcry_md_close(h2);
}

void keypair_print_wif_compressed(KeyPair *key) {
	int i;
	char *wif;
	gcry_md_hd_t h1, h2;
	unsigned char *sha256;
	unsigned char gar[PRIVATE_KEY_COMPRESSED_BYTES + 5];  // 33 plus 4 byte checksum and 1 byte prefix
	
	// Initialize gcrypt libs
	if (!gcry_check_version(GCRYPT_VERSION)) {
		fputs ("libgcrypt version mismatch\n", stderr);
		return;
	}
	
	gcry_md_open(&h1, GCRY_MD_SHA256, 0);
	gcry_md_open(&h2, GCRY_MD_SHA256, 0);
	
	// 0x80 prefix for mainnet, 0xEF for testnet
	gar[0] = 0x80;
	for (i = 0; i < PRIVATE_KEY_COMPRESSED_BYTES; ++i) {
		gar[i+1] = key->private_key_compressed[i];
	}
	
	/*
	 * Private key -> SHA256
	 */
	for (i = 0; i < PRIVATE_KEY_COMPRESSED_BYTES+1; ++i) {
		gcry_md_putc(h1, gar[i]);
	}
	sha256 = gcry_md_read(h1, 0);
	
	/*
	 * SHA256 -> SHA256
	 */
	for (i = 0; i < 32; ++i) {
		gcry_md_putc(h2, sha256[i]);
	}
	sha256 = gcry_md_read(h2, 0);
	
	/*
	 * Get checksum (first 4 bytes)
	 */
	gar[34] = sha256[0];
	gar[35] = sha256[1];
	gar[36] = sha256[2];
	gar[37] = sha256[3];
	
	wif = base58_encode(gar, PRIVATE_KEY_COMPRESSED_BYTES + 5);
	
	printf("%s", wif);
	
	gcry_md_close(h1);
	gcry_md_close(h2);
}

void keypair_print_address_compressed(KeyPair *key) {
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
	for (i = 0; i < PUBLIC_KEY_COMPRESSED_BYTES; ++i) {
		gcry_md_putc(h1, key->public_key_compressed[i]);
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
