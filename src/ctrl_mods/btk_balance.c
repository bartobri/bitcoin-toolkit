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
#include "mods/txoa.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/opts.h"
#include "mods/script.h"
#include "mods/jsonrpc.h"
#include "mods/block.h"
#include "mods/hex.h"
#include "mods/database.h"
#include "mods/transaction.h"
#include "mods/serialize.h"

int btk_balance_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    char address[BUFSIZ];
    char output_str[BUFSIZ];
    uint64_t balance = 0;

    assert(opts);

    (void)input_len;

    if (opts->create || opts->update)
    {
        int i;
        int blockcount = 0;
        int last_block = 0;

        if (opts->update)
        {
            r = txao_get_last_block(&last_block);
            ERROR_CHECK_NEG(r, "Could not get last block processed.");
        }

        r = jsonrpc_init(opts->host_name, opts->host_service, opts->rpc_auth);
        ERROR_CHECK_NEG(r, "Unable to initialize json rpc.");

        r = jsonrpc_get_blockcount(&blockcount);
        ERROR_CHECK_NEG(r, "Could not get block count.");

        for (i = last_block + 1; i <= blockcount; i++)
        {
            uint64_t j;
            char blockhash[BUFSIZ];
            char *block_hex = NULL;
            unsigned char *block_raw = NULL;
            Block block;

            memset(blockhash, 0, BUFSIZ);

            r = jsonrpc_get_blockhash(blockhash, i);
            ERROR_CHECK_NEG(r, "Could not get block hash.");

            r = jsonrpc_get_block(&block_hex, blockhash);
            ERROR_CHECK_NEG(r, "Could not get block data.");

            block_raw = malloc(strlen(block_hex) / 2);
            ERROR_CHECK_NULL(block_raw, "Memory allocation error.");

            r = hex_str_to_raw(block_raw, block_hex);
            ERROR_CHECK_NEG(r, "Could not convert hex block to raw block.");

            block = malloc(sizeof(*block));
            ERROR_CHECK_NULL(block, "Memory allocation error.");

            r = block_from_raw(block, block_raw);
            ERROR_CHECK_NEG(r, "Could not deserialize raw block data.");

            for (j = 0; j < block->tx_count; j++)
            {
                uint32_t k;
                char address[BUFSIZ];

                for (k = 0; k < block->transactions[j]->input_count; k++)
                {
                    memset(address, 0, BUFSIZ);

                    // Skip coinbase inputs. No deduction for them.
                    if (block->transactions[j]->inputs[k]->is_coinbase)
                    {
                        continue;
                    }

                    r = txoa_get(address, block->transactions[j]->inputs[k]->tx_hash, block->transactions[j]->inputs[k]->index);
                    ERROR_CHECK_NEG(r, "Could not get address from txoa database.");

                    if (*address)
                    {
                        // Every input is spent 100% (recouped via change address).
                        // Set all inputs to zero balance.

                        // TODO - Delete address form db instead of setting to zero
                        r = balance_put(address, 0);
                        ERROR_CHECK_NEG(r, "Could not update address balance.");

                        r = txoa_delete(block->transactions[j]->inputs[k]->tx_hash, block->transactions[j]->inputs[k]->index);
                        ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");
                    }
                }

                for (k = 0; k < block->transactions[j]->output_count; k++)
                {
                    memset(address, 0, BUFSIZ);

                    r = script_get_output_address(address,
                                block->transactions[j]->outputs[k]->script_raw,
                                block->transactions[j]->outputs[k]->script_size,
                                block->transactions[j]->version);
                    ERROR_CHECK_NEG(r, "Could not get address from output script.");

                    if (*address)
                    {
                        // TXOA Database
                        r = txoa_put(block->transactions[j]->txid, k, address);
                        ERROR_CHECK_NEG(r, "Could not put entry in the txoa database.");

                        // Balance Database
                        r = balance_put(address, block->transactions[j]->outputs[k]->amount);
                        ERROR_CHECK_NEG(r, "Could not add entry to balance database.");
                    }
                }
            }

            free(block_hex);
            free(block_raw);
            block_free(block);

            r = txao_set_last_block(i);
            ERROR_CHECK_NEG(r, "Could not set last block.");

            printf("Block %i\n", i);
        }
    }
    else
    {
        memset(address, 0, BUFSIZ);
        memset(output_str, 0, BUFSIZ);

        if (opts->input_type_wif)
        {
            r = address_from_wif(address, (char *)input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");
        }
        else if (opts->input_type_string)
        {
            r = address_from_str(address, (char *)input);
            ERROR_CHECK_NEG(r, "Could not calculate address from private key.");
        }
        else
        {
            strcpy(address, (char *)input);
        }

        balance = 0;

        r = balance_get(&balance, address);
        ERROR_CHECK_NEG(r, "Could not query balance database.");

        sprintf(output_str, "%ld", balance);

        *output = output_append_new_copy(*output, output_str, strlen(output_str) + 1);
        ERROR_CHECK_NULL(*output, "Memory allocation error.");
    }

    return EXIT_SUCCESS;
}

int btk_balance_requires_input(opts_p opts)
{
    assert(opts);

    if (opts->create || opts->update)
    {
        return 0;
    }

    return 1;
}

int btk_balance_init(opts_p opts)
{
    int r;

    assert(opts);

    if (opts->create || opts->update)
    {
        ERROR_CHECK_NULL(opts->host_name, "Missing hostname command option.");
        ERROR_CHECK_NULL(opts->rpc_auth, "Missing rpc-auth command option.");
    }

    // TODO - can't use output_path for two different databases. Must fix.

    r = txoa_open(opts->output_path, opts->create);
    ERROR_CHECK_NEG(r, "Could not open txoa database.");

    r = balance_open(opts->output_path, opts->create);
    ERROR_CHECK_NEG(r, "Could not open balance database.");

    return 1;
}

int btk_balance_cleanup(opts_p opts)
{
    assert(opts);

    balance_close();
    txoa_close();

    return 1;
}