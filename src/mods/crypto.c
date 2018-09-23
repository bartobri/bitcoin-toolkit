#include <gcrypt.h>
#include <string.h>
#include "crypto.h"
#include "assert.h"
#include "mem.h"

static int crypto_init(void) {
	static int isInit = 0;
	if (!isInit) {
		if (!gcry_check_version(GCRYPT_VERSION)) {
			return -1;
		}
		isInit = 1;
	}
	return 1;
}

int crypto_get_sha256(unsigned char *output, unsigned char *input, size_t input_len) {
	gcry_md_hd_t gc;
	
	assert(output);
	assert(input);
	assert(input_len);

	if (crypto_init() < 0)
	{
		return -1;
	}
	
	gcry_md_open(&gc, GCRY_MD_SHA256, 0);
	
	gcry_md_write(gc, input, input_len);
	
	memcpy(output, gcry_md_read(gc, 0), 32);
	
	gcry_md_close(gc);
	
	return 1;
}

unsigned char *crypto_get_rmd160(unsigned char *s, size_t l) {
	gcry_md_hd_t gc;
	unsigned char *r;
	
	assert(s);
	assert(l);
	assert(crypto_init());
	
	r = ALLOC(20);
	
	// Initialize
	gcry_md_open(&gc, GCRY_MD_RMD160, 0);
	
	// Crypt
	gcry_md_write(gc, s, l);
	
	// Copy hash
	memcpy(r, gcry_md_read(gc, 0), 20);
	
	// Close gcrypt ADT
	gcry_md_close(gc);
	
	// Return hash
	return r;
}

uint32_t crypto_get_checksum(unsigned char *data, size_t len) {
	int r;
	unsigned char *sha1, *sha2;
	uint32_t checksum = 0;

	sha1 = ALLOC(32);
	sha2 = ALLOC(32);

	r = crypto_get_sha256(sha1, data, len);
	if (r < 0)
	{
		// return negative value
	}
	r = crypto_get_sha256(sha2, sha1, 32);
	if (r < 0)
	{
		// return negative value
	}

	checksum <<= 8;
	checksum += sha2[0];
	checksum <<= 8;
	checksum += sha2[1];
	checksum <<= 8;
	checksum += sha2[2];
	checksum <<= 8;
	checksum += sha2[3];
	
	FREE(sha1);
	FREE(sha2);
	
	return checksum;
}
