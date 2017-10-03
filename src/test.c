#include <stdio.h>
#include "mods/address.h"
#include "mods/base58.h"
#include "mods/base58check.h"
#include "mods/crypto.h"
#include "mods/point.h"
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/random.h"
#include "mods/wif.h"

int main(void) {
	PrivKey priv;
	
	priv = privkey_new();
	
	printf("Private Key: %s\n", privkey_to_hex(priv));

}
