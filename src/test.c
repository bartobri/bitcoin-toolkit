#include <stdio.h>
#include "mods/address.h"
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/wif.h"

int main(void) {
	
	PrivKey priv;
	PubKey pub;
	
	priv = privkey_new();
	pub = pubkey_get(priv);
	
	printf("Private Key: %s\n", privkey_to_hex(priv));
	printf("WIF: %s\n", wif_get(priv));
	printf("Public Key: %s\n", pubkey_to_hex(pub));
	printf("Address: %s\n", address_get(pub));
	
	priv = privkey_compress(priv);
	pub = pubkey_get(priv);
	
	printf("Private Key Compressed: %s\n", privkey_to_hex(priv));
	printf("WIF Compressed: %s\n", wif_get(priv));
	printf("Public Key Compressed: %s\n", pubkey_to_hex(pub));
	printf("Address Compressed: %s\n", address_get(pub));

}
