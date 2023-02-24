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
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include "mods/error.h"
#include "mods/output.h"
#include "mods/balance.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/opts.h"
#include "mods/script.h"
#include "mods/chainstate.h"
#include "mods/utxokey.h"
#include "mods/utxovalue.h"

int btk_balance_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    char address[BUFSIZ];
    char output_str[BUFSIZ];
    PubKey pubkey = NULL;
    uint64_t balance = 0;

    assert(opts);

    (void)output;
    (void)input_len;

    if (opts->create)
    {
        UTXOKey key;
        UTXOValue value;

        r = chainstate_open(opts->input_path);
        ERROR_CHECK_NEG(r, "Could not open chainstate database.");

        r = balance_open(opts->output_path, opts->create);
        ERROR_CHECK_NEG(r, "Could not open address database.");

        r = chainstate_seek_start();
        ERROR_CHECK_NEG(r, "Chainstate seek error.");

        key = malloc(sizeof(struct UTXOKey));
        ERROR_CHECK_NULL(key, "Memory allocation error.");

        value = malloc(sizeof(struct UTXOValue));
        ERROR_CHECK_NULL(value, "Memory allocation error.");

        while ((r = chainstate_get_next(key, value)) > 0)
        {
            memset(address, 0, BUFSIZ);

            if (value->n_size == 0x00)
            {
                r = address_from_rmd160(address, value->script);
                ERROR_CHECK_NEG(r, "Could not generate address from public key hash.");
            }
            else if (value->n_size == 0x01)
            {
                r = address_from_p2sh_script(address, value->script);
                ERROR_CHECK_NEG(r, "Could not generate address from script hash.");
            }
            else if (value->n_size == 0x02 || value->n_size == 0x03)
            {
                pubkey = malloc(pubkey_sizeof());

                r = pubkey_from_raw(pubkey, value->script, value->script_len);
                ERROR_CHECK_NEG(r, "Can not get pubkey object from compressed public key.");

                r = address_get_p2pkh(address, pubkey);
                ERROR_CHECK_NEG(r, "Can not get address from pubkey.");

                free(pubkey);
            }
            else if (value->n_size == 0x04 || value->n_size == 0x05)
            {
                pubkey = malloc(pubkey_sizeof());

                r = pubkey_from_raw(pubkey, value->script, value->script_len);
                ERROR_CHECK_NEG(r, "Can not get pubkey object from uncompressed public key.");

                pubkey_uncompress(pubkey);

                r = address_get_p2pkh(address, pubkey);
                ERROR_CHECK_NEG(r, "Can not get address from pubkey.");

                free(pubkey);
            }
            else
            {
                r = script_get_output_address(address, value->script, value->script_len, 0);
                ERROR_CHECK_NEG(r, "Could not get address from chainstate value.");
            }

            if (*address)
            {
                balance = 0;

                r = balance_get(&balance, address);
                ERROR_CHECK_NEG(r, "Could not query address database.");

                printf("%s (%ld) <- %ld\n", address, balance, value->amount);

                balance += value->amount;

                if (balance > 0)
                {
                    r = balance_put(address, balance);
                    ERROR_CHECK_NEG(r, "Could not create/update record for address database.");
                }
            }

            free(value->script);
            value->script = NULL;
        }
        ERROR_CHECK_NEG(r, "Could not get next chainstate record.");

        utxokey_free(key);
        utxovalue_free(value);

        chainstate_close();
        balance_close();
    }
    else
    {
        r = balance_open(opts->input_path, 0);
        ERROR_CHECK_NEG(r, "Could not open address database.");

        if (opts->input_type_wif)
        {
            memset(address, 0, BUFSIZ);

            r = address_from_wif(address, (char *)input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");
        }
        else if (opts->input_type_string)
        {
            memset(address, 0, BUFSIZ);

            r = address_from_str(address, (char *)input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");
        }
        else
        {
            strcpy(address, (char *)input);
        }

        balance = 0;

        r = balance_get(&balance, address);
        ERROR_CHECK_NEG(r, "Could not query address database.");

        balance_close();

        memset(output_str, 0, BUFSIZ);

        sprintf(output_str, "%ld", balance);

        *output = output_append_new_copy(*output, output_str, strlen(output_str) + 1);
        ERROR_CHECK_NULL(*output, "Memory allocation error.");
    }

    return EXIT_SUCCESS;
}

int btk_balance_requires_input(opts_p opts)
{
    assert(opts);

    if (opts->create)
    {
        return 0;
    }

    return 1;
}