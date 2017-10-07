#include <stdio.h>
#include "mods/address.h"
//#include "mods/base58.h"
//#include "mods/base58check.h"
//#include "mods/crypto.h"
//#include "mods/point.h"
#include "mods/privkey.h"
#include "mods/pubkey.h"
//#include "mods/random.h"
//#include "mods/wif.h"

int main(void) {
	/*
	PrivKey k;
	
	k = privkey_new();
	
	printf("Private Key: %s\n", privkey_to_hex(k));
	
	printf("Private From Hex: %s\n", privkey_to_hex(privkey_from_hex(privkey_to_hex(k))));
	
	printf("Is Compressed: %i\n", privkey_is_compressed(k));
	
	k = privkey_compress(k);

	printf("Compressed Private Key: %s\n", privkey_to_hex(k));
	
	printf("Compressed Private From Hex: %s\n", privkey_to_hex(privkey_from_hex(privkey_to_hex(k))));
	
	printf("Is Compressed: %i\n", privkey_is_compressed(k));
	
	k = privkey_uncompress(k);
	
	printf("Private Key: %s\n", privkey_to_hex(k));
	
	printf("Is Compressed: %i\n", privkey_is_compressed(k));
	*/
	
	PrivKey priv;
	PubKey pub;
	
	priv = privkey_new();
	pub = pubkey_get(priv);
	
	printf("Private Key: %s\n", privkey_to_hex(priv));
	printf("Public Key: %s\n", pubkey_to_hex(pub));
	printf("Address: %s\n", address_get(pub));
	
	priv = privkey_compress(priv);
	pub = pubkey_get(priv);
	
	printf("Private Key Compressed: %s\n", privkey_to_hex(priv));
	printf("Public Key Compressed: %s\n", pubkey_to_hex(pub));
	printf("Address Compressed: %s\n", address_get(pub));

	//printf("WIF: %s\n", wif_get(priv));
	//printf("WIF Compressed: %s\n", wif_get_compressed(priv_comp));

	//printf("Private Key From Hex: %s\n", privkey_to_hex(privkey_from_hex(privkey_to_hex(priv))));
	//printf("Private Key Compressed From Hex: %s\n", privkey_compressed_to_hex(privkey_compressed_from_hex(privkey_compressed_to_hex(priv_comp))));
}
