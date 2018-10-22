/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef NETWORK_H
#define NETWORK_H 1

void network_set_main(void);
void network_set_test(void);
int network_is_main(void);
int network_is_test(void);

#endif
