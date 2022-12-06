/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/input.h"
#include "mods/error.h"

#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_RAW               3
#define INPUT_GUESS             4
#define OUTPUT_P2PKH            1   // Legacy Address
#define OUTPUT_P2WPKH           2   // Segwit (Bech32) Address
#define TRUE                    1
#define FALSE                   0
#define OUTPUT_BUFFER           150

#define INPUT_SET(x)            if (input_format == FALSE) { input_format = x; } else { error_log("Cannot use multiple input format flags."); return -1; }
#define OUTPUT_SET(x)           if (output_format == FALSE) { output_format = x; } else { error_log("Cannot use multiple output format flags."); return -1; }

static int input_format         = FALSE;
static int output_format        = FALSE;

int btk_address_init(int argc, char *argv[])
{
    int o;
    char *command = NULL;

    command = argv[1];

    while ((o = getopt(argc, argv, "whrPW")) != -1)
    {
        switch (o)
        {
            // Input format
            case 'w':
                INPUT_SET(INPUT_WIF)
                break;
            case 'h':
                INPUT_SET(INPUT_HEX)
                break;
            case 'r':
                INPUT_SET(INPUT_RAW);
                break;

            // Output format
            case 'P':
                OUTPUT_SET(OUTPUT_P2PKH);
                break;
            case 'W':
                OUTPUT_SET(OUTPUT_P2WPKH);
                break;

            // Unknown option
            case '?':
                error_log("See 'btk help %s' to read about available argument options.", command);
                if (isprint(optopt))
                {
                    error_log("Invalid command option or argument required: '-%c'.", optopt);
                }
                else
                {
                    error_log("Invalid command option character '\\x%x'.", optopt);
                }
                return -1;
        }
    }

    if (input_format == FALSE)
    {
        input_format = INPUT_GUESS;
    }

    if (output_format == FALSE)
    {
        output_format = OUTPUT_P2PKH;
    }

    return 1;
}

int btk_address_main(void)
{
    int r;
    char *input_sc;
    unsigned char *input_uc;
    char output[OUTPUT_BUFFER];
    PubKey pubkey = NULL;
    PrivKey privkey = NULL;

    pubkey = malloc(pubkey_sizeof());
    if (pubkey == NULL)
    {
        error_log("Memory allocation error");
        return -1;
    }

    privkey = malloc(privkey_sizeof());
    if (privkey == NULL)
    {
        error_log("Memory allocation error");
        return -1;
    }

    switch (input_format)
    {
        case INPUT_WIF:
            r = input_get_str(&input_sc, NULL);
            if (r < 0)
            {
                error_log("Could not get input.");
                return -1;
            }

            r = privkey_from_wif(privkey, input_sc);
            if (r < 0)
            {
                error_log("Could not calculate private key from input.");
                return -1;
            }

            if (privkey_is_zero(privkey))
            {
                error_log("Private key decimal value cannot be zero.");
                return -1;
            }

            r = pubkey_get(pubkey, privkey);
            if (r < 0)
            {
                error_log("Could not calculate public key.");
                return -1;
            }

            free(input_sc);
            break;

        case INPUT_HEX:
            r = input_get_str(&input_sc, NULL);
            if (r < 0)
            {
                error_log("Could not get input.");
                return -1;
            }

            r = pubkey_from_hex(pubkey, input_sc);
            if (r < 0)
            {
                error_log("Could not calculate private key from input.");
                return -1;
            }

            free(input_sc);
            break;

        case INPUT_RAW:
            r = input_get_from_pipe(&input_uc);
            if (r < 0)
            {
                error_log("Could not get input.");
                return -1;
            }

            r = pubkey_from_raw(pubkey, input_uc, r);
            if (r < 0)
            {
                error_log("Could not get public key from input.");
                return -1;
            }

            free(input_uc);
            break;

        case INPUT_GUESS:
            r = input_get_old(&input_uc, NULL, INPUT_GET_MODE_ALL);
            if (r < 0)
            {
                error_log("Could not get input.");
                return -1;
            }

            r = pubkey_from_guess(pubkey, input_uc, r);
            if (r < 0)
            {
                error_log("Could not get public key from input.");
                return -1;
            }

            free(input_uc);
            break;
    }

    if (!pubkey)
    {
        error_log("Could not get public key from input.");
        return -1;
    }

    memset(output, 0, OUTPUT_BUFFER);

    switch (output_format)
    {
        case OUTPUT_P2PKH:
            r = address_get_p2pkh(output, pubkey);
            if (r < 0)
            {
                error_log("Could not calculate P2PKH address.");
                return -1;
            }
            printf("%s\n", output);
            break;

        case OUTPUT_P2WPKH:
            r = address_get_p2wpkh(output, pubkey);
            if (r < 0)
            {
                error_log("Could not calculate P2WPKH address.");
                return -1;
            }
            printf("%s\n", output);
            break;
    }

    free(pubkey);
    free(privkey);

    return 1;
}

int btk_address_cleanup(void)
{
    return 1;
}