#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH      32
#define PRIVKEY_COMP_LENGTH 33

typedef struct {
	unsigned char data[PRIVKEY_COMP_LENGTH];
	int compressed;
} PrivKey;

PrivKey privkey_new(void);
PrivKey privkey_compress(PrivKey);
PrivKey privkey_new_compressed(void);
char   *privkey_to_hex(PrivKey);
PrivKey privkey_from_hex(char *);

#endif
