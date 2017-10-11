#include <stdio.h>
#include <inttypes.h>
//#include "mods/address.h"
//#include "mods/privkey.h"
//#include "mods/pubkey.h"
//#include "mods/wif.h"
#include "mods/txinput.h"
#include "mods/txoutput.h"

int main(void) {
	uint64_t i;
	TXInput t;
	TXOutput x;
	char *raw_input = "818695a4b8902d81fb613b9ff9245ea9dc27dab3543077734626a16e1aacc536000000006a47304402200ac50993e5f9e12e38ded36b1ce603e70423fe5dc5c58c61bd5d7d1fb2a244e702200f56738aaf69a666a27ea8c34b35eca773368d6bcced254c4531930efb8b028d01210330b0a6ef8c22f33ea8d0c400407f0f5de3c4d2118e8ad80eca6114d532aafe4cfeffffff";
	char *raw_output = "fa620205000000001976a9145a110c39dd88e05e697226365a04d16968cb00ed88ac";

	t = txinput_from_rawhex(raw_input);
	printf("INPUT\n");
	printf("TX Hash: ");
	for (i = 0; i < 32; ++i) {
		printf("%02x", t.tx_hash[i]);
	}
	printf("\n");
	printf("TX Output Index: %" PRIu32 "\n", t.index);
	printf("Unlocking Script Size: %" PRIu64 "\n", t.script_size);
	printf("Unlocking Script: ");
	for (i = 0; i < t.script_size; ++i) {
		printf("%02x", t.script[i]);
	}
	printf("\n");
	printf("Sequence: %" PRIu32 "\n", t.sequence);
	
	x = txoutput_from_rawhex(raw_output);
	printf("OUTPUT\n");
	printf("Amount: %" PRIu64 "\n", x.amount);
	printf("Locking Script Size: %" PRIu64 "\n", x.script_size);
	printf("Locking Script: ");
	for (i = 0; i < x.script_size; ++i) {
		printf("%02x", x.script[i]);
	}
	printf("\n");
	
	/*
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
	*/

}
