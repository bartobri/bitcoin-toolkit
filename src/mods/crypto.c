#include <gcrypt.h>
#include <string.h>
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
	
	assert(s);
	assert(l);
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
