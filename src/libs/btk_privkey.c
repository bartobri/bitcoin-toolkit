/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

int btk_privkey_init(int argc, char *argv[]) {
	int o;
	
	// Check arguments
	while ((o = getopt(argc, argv, "g")) != -1) {
		switch (o) {
			case 'g':
				printf("Got g flag\n");
				break;
			case '?':
				if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}
