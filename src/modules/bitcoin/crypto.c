#include <gcrypt.h>

unsigned char *crypto_get_sha256 (unsigned char *s, size_t l) {
	size_t i;
	gcry_md_hd_t gc;
	
	// Initialize
	gcry_md_open(&gc, GCRY_MD_SHA256, 0);
	
	// Crypt
	for (i = 0; i < l; ++i) {
		gcry_md_putc(gc, s[i]);
	}
	
	// Return hash
	return gcry_md_read(gc, 0);
}
