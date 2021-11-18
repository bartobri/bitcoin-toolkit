/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "mods/error.h"
#include "mods/input.h"
#include "mods/addressdb.h"

int btk_addressdb_main(int argc, char *argv[])
{
    int o, r;
    int create = 0;
    uint64_t sats = 0;
    char *input = NULL;
    char *db_path = NULL;

    while ((o = getopt(argc, argv, "p:")) != -1)
    {
        switch (o)
        {
            case 'p':
                db_path = optarg;
                break;
            case 'c':
                create = 1;
                break;
            case '?':
                error_log("See 'btk help database' to read about available argument options.");
                if (isprint(optopt))
                {
                    error_log("Invalid command option '-%c'.", optopt);
                }
                else
                {
                    error_log("Invalid command option character '\\x%x'.", optopt);
                }
                return -1;
        }
    }

    if (create)
    {
        printf("Need code to create database here\n");
        return EXIT_SUCCESS;
    }

    r = input_get_str(&input, "Enter Bitcoin Address: ");
    if (r < 0)
    {
        error_log("Could not get input.");
        return -1;
    }

    r = addressdb_open(db_path);
    if (r < 0)
    {
        error_log("Could not open address database.");
        return -1;
    }

    r = addressdb_get(&sats, input);
    if (r < 0)
    {
        error_log("Could not open address database.");
        return -1;
    }

    printf("Address %s has balance: %"PRIu64"\n", input, sats);

    addressdb_close();

    free(input);

    return EXIT_SUCCESS;
}