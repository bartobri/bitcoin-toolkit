#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH         32
#define PRIVKEY_WIF_LENGTH_MIN 51

typedef struct PrivKey *PrivKey;

PrivKey privkey_new(void);
PrivKey privkey_compress(PrivKey);
PrivKey privkey_uncompress(PrivKey);
PrivKey privkey_new_compressed(void);
char   *privkey_to_hex(PrivKey);
unsigned char *privkey_to_raw(PrivKey, size_t *);
char   *privkey_to_wif(PrivKey);
PrivKey privkey_from_wif(char *);
PrivKey privkey_from_hex(char *);
PrivKey privkey_from_dec(char *);
PrivKey privkey_from_str(char *);
PrivKey privkey_from_raw(unsigned char *, size_t);
PrivKey privkey_from_guess(unsigned char *, size_t);
int     privkey_is_compressed(PrivKey);
void    privkey_free(PrivKey);

#endif
