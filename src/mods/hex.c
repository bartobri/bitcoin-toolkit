
// TODO - handle error if either char is not hex
unsigned char hex_to_dec(char l, char r) {
	unsigned char ret;

	// force lowercase
	if (l >= 'A' && l <= 'F') {
		l -= 55;
	}
	if (r >= 'A' && r <= 'F') {
		r -= 55;
	}
	
	// Deal with numbers
	if (l >= '0' && l <= '9') {
		l -= 48;
	}
	if (r >= '0' && r <= '9') {
		r -= 48;
	}
	
	// Deal with letters
	if (l >= 'a' && l <= 'z') {
		l -= 87;
	}
	if (r >= 'a' && r <= 'z') {
		r -= 87;
	}
	
	ret = (l << 4) + r;
	
	return ret;
}
