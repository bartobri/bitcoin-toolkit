#include <gcrypt.h>
#include <string.h>
#include "crypto.h"
#include "assert.h"
#include "mem.h"

static int isInit = 0;

int crypto_init(void) {
	if (!isInit) {
		if (!gcry_check_version(GCRYPT_VERSION)) {
			return 0;
		}
		isInit = 1;
	}
	return 1;
}

unsigned char *crypto_get_sha256(unsigned char *s, size_t l) {
	gcry_md_hd_t gc;
	unsigned char *r;
	
	if (l) {
		assert(s);
	}
	assert(crypto_init());
	
	r = ALLOC(32);
	
	// Initialize
	gcry_md_open(&gc, GCRY_MD_SHA256, 0);
	
	// Crypt
	gcry_md_write(gc, s, l);
	
	// Copy hash
	memcpy(r, gcry_md_read(gc, 0), 32);
	
	// Close gcrypt ADT
	gcry_md_close(gc);
	
	// Return hash
	return r;
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
	unsigned char *sha1, *sha2;
	uint32_t checksum = 0;

	sha1 = crypto_get_sha256(data, len);
	sha2 = crypto_get_sha256(sha1, 32);

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
