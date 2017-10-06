#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH 32

typedef struct {
	unsigned char data[PRIVKEY_LENGTH + 1];
} PrivKey;

PrivKey privkey_new(void);
PrivKey privkey_compress(PrivKey);
PrivKey privkey_uncompress(PrivKey);
PrivKey privkey_new_compressed(void);
char   *privkey_to_hex(PrivKey);
PrivKey privkey_from_hex(char *);
int     privkey_is_compressed(PrivKey);

#endif
