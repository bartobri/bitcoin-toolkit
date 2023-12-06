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
#include "mods/utxokey.h"
#include "mods/utxovalue.h"
#include "mods/chainstate.h"

#define CHAIN_STATUS_READY    1
#define CHAIN_STATUS_FINAL    2

typedef struct blockchain *blockchain;
struct blockchain {
	int status;
	int block_num;
	Block block;
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
void btk_balance_sleep(void);

int btk_balance_main(output_item *output, opts_p opts, unsigned char *input, size_t input_len)
{
	int r;

	assert(opts);

	(void)input_len;

	if (opts->create_from_chainstate)
	{
		size_t i = 0;
		size_t record_count = 0;
		uint64_t block_height = 0;
		UTXOKey key;
		UTXOValue value;

		printf("Getting record count...");
		fflush(stdout);

		r = chainstate_get_record_count(&record_count);
		ERROR_CHECK_NEG(r, "Could not get chainstate record count.");

		printf("Done.\n");
		fflush(stdout);

		key = malloc(sizeof(*key));
		ERROR_CHECK_NULL(key, "Memory allocation error.");

		value = malloc(sizeof(*value));
		ERROR_CHECK_NULL(key, "Memory allocation error.");

		memset(key, 0, sizeof(*key));
		memset(value, 0, sizeof(*value));

		r = chainstate_seek_start();
		ERROR_CHECK_NEG(r, "Could not set chainstate interator.");

		while ((r = chainstate_get_next(key, value)) > 0)
		{
			uint64_t balance;
			char address[BUFSIZ];
			PubKey pubkey;

			memset(address, 0, BUFSIZ);
			balance = 0;

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
				ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

				r = pubkey_from_raw(pubkey, value->script, value->script_len);
				ERROR_CHECK_NEG(r, "Can not get pubkey object from compressed public key.");

				r = address_get_p2pkh(address, pubkey);
				ERROR_CHECK_NEG(r, "Can not get address from pubkey.");

				free(pubkey);
			}
			else if (value->n_size == 0x04 || value->n_size == 0x05)
			{
				pubkey = malloc(pubkey_sizeof());
				ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

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
				ERROR_CHECK_NEG(r, "Could not get address from utxo script.");
			}

			if (*address)
			{
				r = balance_get(&balance, address);
				ERROR_CHECK_NEG(r, "Could not query balance database.");

				balance += value->amount;

				// TXOA Database
				r = txoa_put(key->tx_hash, key->vout, address);
				ERROR_CHECK_NEG(r, "Could not put entry in the txoa database.");

				// Balance Database
				r = balance_put(address, balance);
				ERROR_CHECK_NEG(r, "Could not add entry to balance database.");
			}

			if (value->height > block_height)
			{
				block_height = value->height;
			}

			free(value->script);
			value->script = NULL;
			
			memset(key, 0, sizeof(*key));
			memset(value, 0, sizeof(*value));

			i++;
			printf("\rBuilding... [%zu/%zu] [%.2f%% Complete]", i, record_count, ((i / (float)record_count) * 100));
			fflush(stdout);
		}
		ERROR_CHECK_NEG(r, "Could not get chainstate record.");

		r = txoa_set_last_block(block_height);
		ERROR_CHECK_NEG(r, "Could not set last block.");

		printf("\n");
		printf("Block height: %"PRId64"\n", block_height);

		utxokey_free(key);
		utxovalue_free(value);
	}
	else if (opts->create || opts->update)
	{
		void *tr;
		thread_args args = NULL;

		args = malloc(sizeof(*args));
		ERROR_CHECK_NULL(args, "Memory allocation error.");

		memset(args, 0, sizeof(*args));

		args->bc_head = NULL;
		args->bc_tail = NULL;
		args->bc_len = 0;

		if (opts->update)
		{
			r = txoa_get_last_block(&(args->last_block));
			ERROR_CHECK_NEG(r, "Could not get last block processed.");
			ERROR_CHECK_FALSE(r, "Could not get last block processed. May need to recreate database.");
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

		free(args);

		printf("\nComplete\n");
	}
	else
	{
		uint64_t balance = 0;
		char address[BUFSIZ];
		char input_str[BUFSIZ];
		char output_str[BUFSIZ];

		memset(address, 0, BUFSIZ);
		memset(input_str, 0, BUFSIZ);
		memcpy(input_str, input, input_len);
		memset(output_str, 0, BUFSIZ);

		if (opts->input_type_wif)
		{
			r = address_from_wif(address, input_str);
			ERROR_CHECK_NEG(r, "Could not calculate address from private key.");
		}
		else if (opts->input_type_string)
		{
			r = address_from_str(address, input_str);
			ERROR_CHECK_NEG(r, "Could not calculate address from private key.");
		}
		else
		{
			strcpy(address, input_str);
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
	int first;

	assert(args);

	first = args->last_block + 1;

	for (i = first; i <= args->block_count; i++)
	{
		char blockhash[BUFSIZ];
		char *block_hex;
		unsigned char *block_raw;
		Block block;

		memset(blockhash, 0, BUFSIZ);

		if (i == first)
		{
			args->bc_head = malloc(sizeof(*(args->bc_head)));
			ERROR_CHECK_NULL(args->bc_head, "Memory allocation error.");

			memset(args->bc_head, 0, sizeof(*(args->bc_head)));

			args->bc_head->block = NULL;
			args->bc_tail = args->bc_head;
			args->bc_tail->next = NULL;
			args->bc_tail->status = 0;
		}
		else
		{
			args->bc_tail->next = malloc(sizeof(*(args->bc_tail->next)));
			ERROR_CHECK_NULL(args->bc_tail->next, "Memory allocation error.");

			memset(args->bc_tail->next, 0, sizeof(*(args->bc_tail->next)));

			args->bc_tail->next->block = NULL;
			args->bc_tail = args->bc_tail->next;
			args->bc_tail->next = NULL;
			args->bc_tail->status = 0;
		}

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

		args->bc_len += 1;
		args->bc_tail->block_num = i;
		args->bc_tail->block = block;

		if (i == args->block_count)
		{
			args->bc_tail->status = CHAIN_STATUS_FINAL;
		}
		else
		{
			args->bc_tail->status = CHAIN_STATUS_READY;
		}

		free(block_raw);
		free(block_hex);

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

	while (1)
	{
		uint64_t i;
		Block block;
		blockchain tmp;
		int status;
		int block_num;

		while (args->bc_head == NULL || !args->bc_head->status)
		{
			btk_balance_sleep();
		}

		status = args->bc_head->status;
		block = args->bc_head->block;
		block_num = args->bc_head->block_num;

		for (i = 0; i < block->tx_count; i++)
		{
			uint32_t j;
			char address[BUFSIZ];
			uint64_t amount;
			uint64_t prev_balance;

			for (j = 0; j < block->transactions[i]->input_count; j++)
			{
				amount = 0;
				memset(address, 0, BUFSIZ);

				// Skip coinbase inputs. No deduction for them.
				if (block->transactions[i]->inputs[j]->is_coinbase)
				{
					continue;
				}

				r = txoa_get(address, &amount, block->transactions[i]->inputs[j]->tx_hash, block->transactions[i]->inputs[j]->index);
				ERROR_CHECK_NEG(r, "Could not get address from txoa database.");

				if (*address)
				{
					prev_balance = 0;

					// Every input is spent 100% (recouped via change address).

					r = txoa_batch_delete(block->transactions[i]->inputs[j]->tx_hash, block->transactions[i]->inputs[j]->index);
					ERROR_CHECK_NEG(r, "Could not delete txao entry after spending.");

					// Get previous balance (if any)
					r = balance_get(&prev_balance, address);
					ERROR_CHECK_NEG(r, "Could not query balance database.");

					if (prev_balance > amount)
					{
						r = balance_batch_put(address, prev_balance - amount);
						ERROR_CHECK_NEG(r, "Could not add entry to balance database.");
					}
					else
					{
						r = balance_batch_delete(address);
						ERROR_CHECK_NEG(r, "Could not update address balance.");
					}
				}
			}

			r = txoa_batch_write();
			ERROR_CHECK_NEG(r, "Could not batch write txao records.");

			r = balance_batch_write();
			ERROR_CHECK_NEG(r, "Could not batch write balance records.");

			for (j = 0; j < block->transactions[i]->output_count; j++)
			{
				memset(address, 0, BUFSIZ);

				r = script_get_output_address(address,
							block->transactions[i]->outputs[j]->script_raw,
							block->transactions[i]->outputs[j]->script_size,
							block->transactions[i]->version);
				ERROR_CHECK_NEG(r, "Could not get address from output script.");

				amount = block->transactions[i]->outputs[j]->amount;

				if (*address && amount > 0)
				{
					prev_balance = 0;

					// TXOA Database
					r = txoa_batch_put(block->transactions[i]->txid, j, address, amount);
					ERROR_CHECK_NEG(r, "Could not put entry in the txoa database.");

					// Get previous balance (if any)
					r = balance_get(&prev_balance, address);
					ERROR_CHECK_NEG(r, "Could not query balance database.");

					// Balance Database
					r = balance_batch_put(address, amount + prev_balance);
					ERROR_CHECK_NEG(r, "Could not add entry to balance database.");
				}
			}

			r = txoa_batch_write();
			ERROR_CHECK_NEG(r, "Could not batch write txao records.");

			r = balance_batch_write();
			ERROR_CHECK_NEG(r, "Could not batch write balance records.");
		}

		r = txoa_set_last_block(block_num);
		ERROR_CHECK_NEG(r, "Could not set last block.");

		if (status == CHAIN_STATUS_FINAL)
		{
			break;
		}

		args->bc_len -= 1;

		printf("\rUpdating... [Block %i/%i] [%.2f%% Complete]", block_num, args->block_count, ((args->bc_head->block_num / (float)args->block_count) * 100));
		fflush(stdout);

		while(args->bc_head->next == NULL)
		{
			btk_balance_sleep();
		}

		tmp = args->bc_head;
		args->bc_head = args->bc_head->next;
		block_free(tmp->block);
		free(tmp);
	}

	return 1;
}

void btk_balance_sleep(void)
{
	struct timespec bcsleep;
	bcsleep.tv_sec = 0;
	bcsleep.tv_nsec = 100000000; // 0.1 seconds

	nanosleep(&bcsleep, NULL);
}

int btk_balance_requires_input(opts_p opts)
{
	assert(opts);

	if (opts->create || opts->create_from_chainstate || opts->update)
	{
		return 0;
	}

	return 1;
}

int btk_balance_init(opts_p opts)
{
	int r;

	assert(opts);

	// Option sanity check.
	int i = 0;
	if (opts->create) { i++; }
	if (opts->create_from_chainstate) { i++; }
	if (opts->update) { i++; }
	ERROR_CHECK_TRUE((i > 1), "Cannot use more than one create or update option.");

	if (opts->create || opts->update)
	{
		ERROR_CHECK_NULL(opts->host_name, "Missing hostname command option.");
		ERROR_CHECK_NULL(opts->rpc_auth, "Missing rpc-auth command option.");
	}

	if (opts->create_from_chainstate)
	{
		r = chainstate_open(opts->chainstate_path);
		ERROR_CHECK_NEG(r, "Could not open txoa database.");
	}

	r = balance_open(opts->balance_path, (opts->create || opts->create_from_chainstate));
	ERROR_CHECK_NEG(r, "Could not open balance database.");

	if (opts->create || opts->create_from_chainstate || opts->update)
	{
		char *txoa_path = NULL;

		if (opts->balance_path)
		{
			txoa_path = malloc(BUFSIZ);
			ERROR_CHECK_NULL(txoa_path, "Memory allocation error.");

			strcpy(txoa_path, opts->balance_path);
			strcat(txoa_path, "/txoa/");
		}

		r = txoa_open(txoa_path, (opts->create || opts->create_from_chainstate));
		ERROR_CHECK_NEG(r, "Could not open txoa database.");

		if (txoa_path)
		{
			free(txoa_path);
		}
	}

	return 1;
}

int btk_balance_cleanup(opts_p opts)
{
	assert(opts);

	if (opts->create_from_chainstate)
	{
		chainstate_close();
	}

	if (opts->create || opts->update)
	{
		txoa_close();
	}

	balance_close();

	return 1;
}