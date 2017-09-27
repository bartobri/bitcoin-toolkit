#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH 32

typedef struct {
	unsigned char data[PRIVKEY_LENGTH];
} PrivKey;

PrivKey privkey_new(void);

#endif
