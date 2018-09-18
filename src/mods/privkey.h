#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH         32
#define PRIVKEY_WIF_LENGTH_MIN 51
#define PRIVKEY_WIF_LENGTH_MAX 52

typedef struct PrivKey *PrivKey;

int privkey_new(PrivKey);
int privkey_compress(PrivKey);
int privkey_uncompress(PrivKey);
int privkey_to_hex(char *, PrivKey);
int privkey_to_raw(unsigned char *, PrivKey);
int privkey_to_wif(char *, PrivKey);
int privkey_from_wif(PrivKey, char *);
int privkey_from_hex(PrivKey, char *);
int privkey_from_dec(PrivKey, char *);
int privkey_from_str(PrivKey, char *);
int privkey_from_raw(PrivKey, unsigned char *, size_t);
PrivKey privkey_from_blob(unsigned char *, size_t);
PrivKey privkey_from_guess(unsigned char *, size_t);
int     privkey_is_compressed(PrivKey);
int     privkey_is_zero(PrivKey);
void    privkey_free(PrivKey);
size_t  privkey_sizeof(void);

#endif
