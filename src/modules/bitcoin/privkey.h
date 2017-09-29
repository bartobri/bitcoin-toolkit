#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH      32
#define PRIVKEY_COMP_LENGTH 33

typedef struct {
	unsigned char data[PRIVKEY_LENGTH];
} PrivKey;

typedef struct {
	unsigned char data[PRIVKEY_COMP_LENGTH];
} PrivKeyComp;

PrivKey     privkey_new(void);
PrivKeyComp privkey_compress(PrivKey);

#endif
