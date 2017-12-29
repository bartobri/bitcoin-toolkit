/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/btk_privkey.h"
#include "libs/btk_pubkey.h"

int main(int argc, char *argv[]) {

	if (argc <= 1) {
		fprintf(stderr, "Missing command argument\n");
		return EXIT_FAILURE;
	}
	
	if (strcmp(argv[1], "privkey") == 0) {
		return btk_privkey_main(argc, argv);
	} else if (strcmp(argv[1], "pubkey") == 0) {
		return btk_pubkey_main(argc, argv);
	} else {
		fprintf(stderr, "Unknown Command: %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
