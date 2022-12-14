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
#include "mods/json.h"
#include "mods/error.h"

#define INPUT_ASCII             1
#define INPUT_WIF               1
#define INPUT_HEX               2
#define INPUT_GUESS             3
#define OUTPUT_P2PKH            1   // Legacy Address
#define OUTPUT_P2WPKH           2   // Segwit (Bech32) Address
#define TRUE                    1
#define FALSE                   0
#define OUTPUT_BUFFER           150

#define INPUT_SET_FORMAT(x)     if (input_format == FALSE) { input_format = x; } else { error_log("Cannot specify ascii input format."); return -1; }
#define INPUT_SET(x)            if (input_type == FALSE) { input_type = x; } else { error_log("Cannot use multiple input format flags."); return -1; }
#define OUTPUT_SET(x)           if (output_type == FALSE) { output_type = x; } else { error_log("Cannot use multiple output format flags."); return -1; }

int btk_address_input_to_json(unsigned char **, size_t *);

static int input_format         = FALSE;
static int input_type           = FALSE;
static int output_type          = FALSE;

int btk_address_init(int argc, char *argv[])
{
    int o;
    char *command = NULL;

    command = argv[1];

    while ((o = getopt(argc, argv, "awhPW")) != -1)
    {
        switch (o)
        {
            // Input format
            case 'a':
                INPUT_SET_FORMAT(INPUT_ASCII);
                break;

            // Input type
            case 'w':
                INPUT_SET(INPUT_WIF)
                break;
            case 'h':
                INPUT_SET(INPUT_HEX)
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

    if (input_type == FALSE)
    {
        input_type = INPUT_GUESS;
    }

    if (output_type == FALSE)
    {
        output_type = OUTPUT_P2PKH;
    }

    return 1;
}

int btk_address_main(void)
{
    int r;
    //char *input_sc;
    //unsigned char *input_uc;
    //char output[OUTPUT_BUFFER];
    
    size_t i, len, input_len;
    unsigned char *input; 
    char input_str[BUFSIZ];
    char output_str[BUFSIZ];
    PubKey pubkey = NULL;
    PrivKey privkey = NULL;

    memset(input_str, 0, BUFSIZ);
    memset(output_str, 0, BUFSIZ);

    privkey = malloc(privkey_sizeof());
    ERROR_CHECK_NULL(privkey, "Memory allocation error.");

    pubkey = malloc(pubkey_sizeof());
    ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

    json_init();

    r = input_get(&input, &input_len);
    ERROR_CHECK_NEG(r, "Error getting input.");

    if (input_format == INPUT_ASCII)
    {
        r = btk_address_input_to_json(&input, &input_len);
        ERROR_CHECK_NEG(r, "Could not convert input to JSON.");
    }

    if(json_is_valid((char *)input, input_len))
    {
        r = json_set_input((char *)input);
        ERROR_CHECK_NEG(r, "Could not load JSON input.");

        r = json_get_input_len((int *)&len);
        ERROR_CHECK_NEG(r, "Could not get input list length.");

        for (i = 0; i < len; i++)
        {
            memset(input_str, 0, BUFSIZ);

            r = json_get_input_index(input_str, BUFSIZ, i);
            ERROR_CHECK_NEG(r, "Could not get JSON string object at index.");

            switch (input_type)
            {
                case INPUT_WIF:
                    r = privkey_from_wif(privkey, input_str);
                    ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
                    r = pubkey_get(pubkey, privkey);
                    ERROR_CHECK_NEG(r, "Could not calculate public key.");
                    break;
                case INPUT_HEX:
                    r = pubkey_from_hex(pubkey, input_str);
                    ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
                    break;
                case INPUT_GUESS:
                    r = pubkey_from_guess(pubkey, (unsigned char *)input_str, strlen(input_str));
                    ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
                    break;
            }

            switch (output_type)
            {
                case OUTPUT_P2PKH:
                    r = address_get_p2pkh(output_str, pubkey);
                    ERROR_CHECK_NEG(r, "Could not calculate P2PKH address.");
                    break;
                case OUTPUT_P2WPKH:
                    r = address_get_p2wpkh(output_str, pubkey);
                    ERROR_CHECK_NEG(r, "Could not calculate P2WPKH address.");
                    break;
            }

            r = json_add(output_str);
            ERROR_CHECK_NEG(r, "Error while generating JSON.");
        }
    }
    else
    {
        error_log("Invalid JSON. Input must be in JSON format or specify a non-JSON input format.");
        return -1;
    }

    json_print();
    json_free();

    free(pubkey);
    free(privkey);

    return 1;
}

int btk_address_cleanup(void)
{
    return 1;
}

int btk_address_input_to_json(unsigned char **input, size_t *input_len)
{
    int r;
    size_t i;
    char str[BUFSIZ];

    memset(str, 0, BUFSIZ);

    for (i = 0; i < *input_len; i++)
    {
        if (i > BUFSIZ-1)
        {
            error_log("Input string too large. Consider using -b for arbitrarily large amounts of input data.");
            return -1;
        }
        if (isascii((*input)[i]))
        {
            str[i] = (*input)[i];
        }
        else
        {
            error_log("Input contains non-ascii characters.");
            return -1;
        }
    }

    while (str[(*input_len)-1] == '\n' || str[(*input_len)-1] == '\r')
    {
        str[(*input_len)-1] = '\0';
        (*input_len)--;
    }

    // Free input here because json_from_string reallocates it.
    free(*input);

    r = json_from_string((char **)input, str);
    ERROR_CHECK_NEG(r, "Could not convert input to JSON.");

    *input_len = strlen((char *)*input);

    return 1;
}