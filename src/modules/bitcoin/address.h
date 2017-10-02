#ifndef ADDRESS_H
#define ADDRESS_H 1

#include "pubkey.h"

char *address_get(PubKey);
char *address_get_compressed(PubKeyComp);

#endif
