#include <stdio.h>
//#include "mods/address.h"
//#include "mods/base58.h"
//#include "mods/base58check.h"
//#include "mods/crypto.h"
//#include "mods/point.h"
#include "mods/privkey.h"
//#include "mods/pubkey.h"
//#include "mods/random.h"
//#include "mods/wif.h"

int main(void) {
	PrivKey k;
	
	k = privkey_new();
	
	printf("Private Key: %s\n", privkey_to_hex(k));
	
	printf("Private From Hex: %s\n", privkey_to_hex(privkey_from_hex(privkey_to_hex(k))));
	
	k = privkey_compress(k);

	printf("Compressed Private Key: %s\n", privkey_to_hex(k));
	
	printf("Compressed Private From Hex: %s\n", privkey_to_hex(privkey_from_hex(privkey_to_hex(k))));
	
	/*
	PrivKey priv;
	PrivKeyComp priv_comp;
	PubKey pub;
	PubKeyComp pub_comp;
	
	priv = privkey_new();
	priv_comp = privkey_compress(priv);
	pub = pubkey_get(priv);
	pub_comp = pubkey_compress(pub);
	
	printf("Private Key: %s\n", privkey_to_hex(priv));
	printf("Private Key Compressed: %s\n", privkey_compressed_to_hex(priv_comp));
	printf("Public Key: %s\n", pubkey_to_hex(pub));
	printf("Public Key Compressed: %s\n", pubkey_compressed_to_hex(pub_comp));
	printf("Address: %s\n", address_get(pub));
	printf("Address Compressed: %s\n", address_get_compressed(pub_comp));
	printf("WIF: %s\n", wif_get(priv));
	printf("WIF Compressed: %s\n", wif_get_compressed(priv_comp));

	printf("Private Key From Hex: %s\n", privkey_to_hex(privkey_from_hex(privkey_to_hex(priv))));
	printf("Private Key Compressed From Hex: %s\n", privkey_compressed_to_hex(privkey_compressed_from_hex(privkey_compressed_to_hex(priv_comp))));
	*/
}
