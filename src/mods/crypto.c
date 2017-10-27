#include <gcrypt.h>
#include "assert.h"

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
	
	assert(s);
	assert(l);
	assert(crypto_init());
	
	// Initialize
	gcry_md_open(&gc, GCRY_MD_SHA256, 0);
	
	// Crypt
	gcry_md_write(gc, s, l);
	
	// Return hash
	// TODO - Don't return the digest directly. Copy it to another string
	// and return it. Then free up the gcrypt handle.
	return gcry_md_read(gc, 0);
}

unsigned char *crypto_get_rmd160(unsigned char *s, size_t l) {
	gcry_md_hd_t gc;
	
	assert(s);
	assert(l);
	assert(crypto_init());
	
	// Initialize
	gcry_md_open(&gc, GCRY_MD_RMD160, 0);
	
	// Crypt
	gcry_md_write(gc, s, l);
	
	// Return hash
	// TODO - Don't return the digest directly. Copy it to another string
	// and return it. Then free up the gcrypt handle.
	return gcry_md_read(gc, 0);
}
