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
#include <time.h>
#include <assert.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/input.h"
#include "mods/base58.h"
#include "mods/base32.h"
#include "mods/json.h"
#include "mods/opts.h"
#include "mods/error.h"

int btk_address_vanity_match(char *, char *);
int btk_address_get_vanity_estimate(long int *, long int);

static int input_format    = OPTS_INPUT_FORMAT_JSON;
static int input_type      = OPTS_INPUT_TYPE_HEX;
static int output_type     = OPTS_OUTPUT_TYPE_P2PKH;

int btk_address_main(opts_p opts)
{
    int r;
    size_t i, len, input_len;
    unsigned char *input; 
    char input_str[BUFSIZ];
    char output_str[BUFSIZ];
    char output_str2[BUFSIZ];
    PubKey pubkey = NULL;
    PrivKey privkey = NULL;

    assert(opts);

    // Override defaults
    if (opts->input_format) { input_format = opts->input_format; }
    if (opts->input_type) { input_type = opts->input_type; }
    if (opts->output_type) { output_type = opts->output_type; }

    memset(input_str, 0, BUFSIZ);
    memset(output_str, 0, BUFSIZ);
    memset(output_str2, 0, BUFSIZ);

    privkey = malloc(privkey_sizeof());
    ERROR_CHECK_NULL(privkey, "Memory allocation error.");

    pubkey = malloc(pubkey_sizeof());
    ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

    json_init();

    r = input_get(&input, &input_len);
    ERROR_CHECK_NEG(r, "Error getting input.");

    if (input_format == OPTS_OUTPUT_FORMAT_ASCII)
    {
        r = json_from_input(&input, &input_len);
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
                case OPTS_INPUT_TYPE_WIF:
                    r = privkey_from_wif(privkey, input_str);
                    ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
                    r = pubkey_get(pubkey, privkey);
                    ERROR_CHECK_NEG(r, "Could not calculate public key.");
                    break;
                case OPTS_INPUT_TYPE_HEX:
                    r = pubkey_from_hex(pubkey, input_str);
                    ERROR_CHECK_NEG(r, "Could not calculate public key from input.");
                    break;
                case OPTS_INPUT_TYPE_VANITY:
                    r = privkey_new(privkey);
                    ERROR_CHECK_NEG(r, "Could not generate a new private key.");
                    r = pubkey_get(pubkey, privkey);
                    ERROR_CHECK_NEG(r, "Could not calculate public key.");
                    break;
                default:
                    r = pubkey_from_guess(pubkey, (unsigned char *)input_str, strlen(input_str));
                    if (r < 0)
                    {
                        error_clear();
                        ERROR_CHECK_NEG(-1, "Invalid of missing input type specified.");
                    }
                    break;
            }

            switch (output_type)
            {
                case OPTS_OUTPUT_TYPE_P2PKH:
                    r = address_get_p2pkh(output_str, pubkey);
                    ERROR_CHECK_NEG(r, "Could not calculate P2PKH address.");
                    break;
                case OPTS_OUTPUT_TYPE_P2WPKH:
                    r = address_get_p2wpkh(output_str, pubkey);
                    ERROR_CHECK_NEG(r, "Could not calculate P2WPKH address.");
                    break;
                default:
                    ERROR_CHECK_NEG(-1, "Invalid output type specified.");
                    break;
            }

            if (input_type == OPTS_INPUT_TYPE_VANITY)
            {
                r = btk_address_vanity_match(input_str, output_str);
                ERROR_CHECK_NEG(r, "Error matching vanity string.");
                if (r == 0)
                {
                    i--;
                    continue;
                }

                r = privkey_to_wif(output_str2, privkey);
                ERROR_CHECK_NEG(r, "Could not convert private key to WIF format.");
                r = json_add(output_str2);
                ERROR_CHECK_NEG(r, "Error while generating JSON.");
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

int btk_address_vanity_match(char *input_str, char *output_str)
{
    int r, offset;
    size_t i, input_len;
    long int perms_total = 0, estimate;
    static long int perms_checked = 0;

    input_len = strlen(input_str);

    switch (output_type)
    {
        case OPTS_OUTPUT_TYPE_P2PKH:
            for (i = 0; i < input_len; ++i)
            {
                r = base58_ischar(input_str[i]);
                ERROR_CHECK_FALSE(r, "Input error. Must only contain base58 characters.");
            }
            offset = 1;
            break;
        case OPTS_OUTPUT_TYPE_P2WPKH:
            for (i = 0; i < input_len; ++i)
            {
                r = base32_get_raw(input_str[i]);
                ERROR_CHECK_NEG(r, "Input error.");
            }
            offset = 4;
            break;
    }

    perms_total = 1;
    for (i = 0; i < input_len; ++i)
    {
        if (isalpha(input_str[i]))
        {
            perms_total *= 29;
        }
        else
        {
            perms_total *= 58;
        }
    }

    r = btk_address_get_vanity_estimate(&estimate, perms_total);
    ERROR_CHECK_NEG(r, "Error computing estimated time left.");

    if (strncasecmp(input_str, output_str + offset, input_len) == 0)
    {
        return 1;
    }

    perms_checked++;
        
    if (perms_checked % 10000 == 0)
    {
        r = btk_address_get_vanity_estimate(&estimate, perms_checked);
        ERROR_CHECK_NEG(r, "Error computing estimated time left.");
        
        fprintf(stderr, "Estimated Seconds Remaining: %ld\n", estimate);
        fflush(stderr);
    }

    return 0;
}

int btk_address_get_vanity_estimate(long int *e, long int p)
{
    long double estimate;
    static long int perms = 0;
    static time_t start_time = 0;
    time_t elapsed_time = 0;

    if (!perms)
    {
        perms = p;
        start_time = time(NULL);
        return 1;
    }
    
    ERROR_CHECK_FALSE(perms, "Can not generate time estimate. Did not initialize permutations.");

    elapsed_time = time(NULL) - start_time;

    estimate = (long double)perms / (long double)p;
    estimate = (estimate * (long double)elapsed_time);
    estimate -= elapsed_time;
    if (estimate < 0)
    {
        *e = 0;
    }
    else
    {
        *e = (long int)estimate;
    }

    return 1;
}