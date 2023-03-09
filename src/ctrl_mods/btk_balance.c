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
#include <time.h>
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

typedef struct blockchain *blockchain;
struct blockchain {
    int pready;
    int block_num;
    char *block_hex;
    blockchain next;
};

typedef struct thread_args *thread_args;
struct thread_args {
    int last_block;
    int block_count;
    blockchain bc_head;
    blockchain bc_tail;
    int bc_len;
};

static pthread_t download_thread;
static pthread_t process_thread;

void *btk_balance_pthread(void *);
int btk_balance_download(thread_args);
int btk_balance_process(thread_args);

int btk_balance_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    void *tr;
    char address[BUFSIZ];
    char output_str[BUFSIZ];
    uint64_t balance = 0;

    assert(opts);

    (void)input_len;

    if (opts->create || opts->update)
    {
        thread_args args = NULL;

        args = malloc(sizeof(*args));
        ERROR_CHECK_NULL(args, "Memory allocation error.");

        memset(args, 0, sizeof(*args));

        args->bc_head = NULL;
        args->bc_tail = NULL;

        if (opts->update)
        {
            r = txao_get_last_block(&(args->last_block));
            ERROR_CHECK_NEG(r, "Could not get last block processed.");
        }

        r = jsonrpc_init(opts->host_name, opts->host_service, opts->rpc_auth);
        ERROR_CHECK_NEG(r, "Unable to initialize json rpc.");

        r = jsonrpc_get_blockcount(&(args->block_count));
        ERROR_CHECK_NEG(r, "Could not get block count.");

        r = pthread_create(&download_thread, NULL, &btk_balance_pthread, args);
        ERROR_CHECK_TRUE(r > 0, "Could not create download thread.");

        r = pthread_create(&process_thread, NULL, &btk_balance_pthread, args);
        ERROR_CHECK_TRUE(r > 0, "Could not create download thread.");

        r = pthread_join(download_thread, &tr);
        ERROR_CHECK_TRUE(r > 0, "Could not join download thread.");

        if (tr != PTHREAD_CANCELED)
        {
            r = *(int *)tr;
            ERROR_CHECK_NEG(r, "Download thread error.");
        }

        r = pthread_join(process_thread, &tr);
        ERROR_CHECK_TRUE(r > 0, "Could not join process thread.");

        if (tr != PTHREAD_CANCELED)
        {
            r = *(int *)tr;
            ERROR_CHECK_NEG(r, "Process thread error.");
        }

        printf("Compete\n");
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

void *btk_balance_pthread(void *args_in)
{
    static int r;
    static int rd;
    static int rp;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    if (pthread_equal(download_thread, pthread_self()))
    {
        rd = btk_balance_download((thread_args)args_in);
        if (rd < 0)
        {
            pthread_cancel(process_thread);
            return &rd;
        }
    }
    else if (pthread_equal(process_thread, pthread_self()))
    {
        rp = btk_balance_process((thread_args)args_in);
        if (rp < 0)
        {
            pthread_cancel(download_thread);
            return &rp;
        }
    }

    r = 1;
    return &r;
}

int btk_balance_download(thread_args args)
{
    int i;
    int r;

    assert(args);

    for (i = args->last_block + 1; i <= args->block_count; i++)
    {
        char blockhash[BUFSIZ];

        memset(blockhash, 0, BUFSIZ);

        r = jsonrpc_get_blockhash(blockhash, i);
        ERROR_CHECK_NEG(r, "Could not get block hash.");

        if (args->bc_head == NULL)
        {
            args->bc_head = malloc(sizeof(*(args->bc_head)));
            ERROR_CHECK_NULL(args->bc_head, "Memory allocation error.");

            memset(args->bc_head, 0, sizeof(*(args->bc_head)));

            args->bc_head->block_hex = NULL;
            args->bc_tail = args->bc_head;
            args->bc_tail->next = NULL;
        }
        else
        {
            args->bc_tail->next = malloc(sizeof(*(args->bc_tail->next)));
            ERROR_CHECK_NULL(args->bc_tail->next, "Memory allocation error.");

            memset(args->bc_tail->next, 0, sizeof(*(args->bc_tail->next)));

            args->bc_tail->next->block_hex = NULL;
            args->bc_tail = args->bc_tail->next;
            args->bc_tail->next = NULL;
        }

        r = jsonrpc_get_block(&(args->bc_tail->block_hex), blockhash);
        ERROR_CHECK_NEG(r, "Could not get block data.");

        args->bc_len += 1;
        args->bc_tail->block_num = i;
        args->bc_tail->pready = 1;

        printf("Downloaded Block %i\n", i);

        while (args->bc_len >= 100)
        {
            sleep(1);
        }
    }

    return 1;
}

int btk_balance_process(thread_args args)
{
    int r;

    assert(args);

    while (1)
    {
        uint64_t i;
        unsigned char *block_raw = NULL;
        Block block;
        blockchain tmp;

        while (args->bc_head == NULL || !args->bc_head->pready)
        {
            struct timespec bcsleep;
            bcsleep.tv_sec = 0;
            bcsleep.tv_nsec = 100000000; // 0.1 seconds

            nanosleep(&bcsleep, NULL);
        }

        block_raw = malloc(strlen(args->bc_head->block_hex) / 2);
        ERROR_CHECK_NULL(block_raw, "Memory allocation error.");

        r = hex_str_to_raw(block_raw, args->bc_head->block_hex);
        ERROR_CHECK_NEG(r, "Could not convert hex block to raw block.");

        block = malloc(sizeof(*block));
        ERROR_CHECK_NULL(block, "Memory allocation error.");

        r = block_from_raw(block, block_raw);
        ERROR_CHECK_NEG(r, "Could not deserialize raw block data.");

        for (i = 0; i < block->tx_count; i++)
        {
            uint32_t j;
            char address[BUFSIZ];

            for (j = 0; j < block->transactions[i]->input_count; j++)
            {
                memset(address, 0, BUFSIZ);

                // Skip coinbase inputs. No deduction for them.
                if (block->transactions[i]->inputs[j]->is_coinbase)
                {
                    continue;
                }

                r = txoa_get(address, block->transactions[i]->inputs[j]->tx_hash, block->transactions[i]->inputs[j]->index);
                ERROR_CHECK_NEG(r, "Could not get address from txoa database.");

                if (*address)
                {
                    // Every input is spent 100% (recouped via change address).
                    // Set all inputs to zero balance.

                    // TODO - Delete address form db instead of setting to zero
                    r = balance_put(address, 0);
                    ERROR_CHECK_NEG(r, "Could not update address balance.");

                    r = txoa_delete(block->transactions[i]->inputs[j]->tx_hash, block->transactions[i]->inputs[j]->index);
                    ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");
                }
            }

            for (j = 0; j < block->transactions[i]->output_count; j++)
            {
                memset(address, 0, BUFSIZ);

                r = script_get_output_address(address,
                            block->transactions[i]->outputs[j]->script_raw,
                            block->transactions[i]->outputs[j]->script_size,
                            block->transactions[i]->version);
                ERROR_CHECK_NEG(r, "Could not get address from output script.");

                if (*address)
                {
                    // TXOA Database
                    r = txoa_put(block->transactions[i]->txid, j, address);
                    ERROR_CHECK_NEG(r, "Could not put entry in the txoa database.");

                    // Balance Database
                    r = balance_put(address, block->transactions[i]->outputs[j]->amount);
                    ERROR_CHECK_NEG(r, "Could not add entry to balance database.");
                }
            }
        }

        r = txao_set_last_block(i);
        ERROR_CHECK_NEG(r, "Could not set last block.");

        printf("Processed Block %i\n", args->bc_head->block_num);

        tmp = args->bc_head->next;
        free(args->bc_head->block_hex);
        free(args->bc_head);
        args->bc_head = tmp;

        args->bc_len -= 1;

        free(block_raw);
        block_free(block);
    }

    return 1;
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