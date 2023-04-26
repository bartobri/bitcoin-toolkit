/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include "mods/node.h"
#include "mods/error.h"
#include "mods/json.h"
#include "mods/cJSON/cJSON.h"

#define JSONRPC_START    "POST / HTTP/1.0\r\n"
#define JSONRPC_HOST     "Host: %s\r\n"
#define JSONRPC_AUTH     "Authorization: Basic %s\r\n"
#define JSONRPC_CONTYPE  "Content-type: text/plain;\r\n"
#define JSONRPC_CONLEN   "Content-length: %ld\r\n"
#define JSONRPC_DATA_START "\r\n"
#define JSONRPC_DATA     "{\"jsonrpc\": \"1.0\", \"id\":\"rpcrequest\", \"method\":\"%s\", \"params\":[%s] }"
#define JSONRPC_DATA_END "\r\n"

#define JSONRPC_DEFAULT_PORT "8332"
#define JSONRPC_ERROR_KEY "error"
#define JSONRPC_RESULT_KEY "result"

static char *hostname    = NULL;
static char *service     = NULL;
static char *auth_str    = NULL;

int jsonrpc_build_request(char *, int, char *, char *);
int jsonrpc_send_request(unsigned char **, char *);
int jsonrpc_parse_response(cJSON **, char *);
int jsonrpc_has_error(cJSON *);

int jsonrpc_init(char *hn, char *serv, char *auth)
{
	assert(hn);
	assert(auth);

	hostname = malloc(strlen(hn) + 1);
	ERROR_CHECK_NULL(hostname, "Memory allocation error.")
	strcpy(hostname, hn);

	if (serv)
	{
		service = serv;
	}
	else
	{
		service = JSONRPC_DEFAULT_PORT;
	}

	// auth = base64(username:password)
	auth_str = malloc(strlen(auth) + 1);
	ERROR_CHECK_NULL(auth_str, "Memory allocation error.")
	strcpy(auth_str, auth);

	return 1;
}

int jsonrpc_get_blockcount(int *count)
{
	int r;
	char json_request[BUFSIZ];
	unsigned char *response;
	cJSON *jobj = NULL;
	cJSON *result;

	assert(count);

	memset(json_request, 0, BUFSIZ);

	r = jsonrpc_build_request(json_request, BUFSIZ - 1, "getblockcount", NULL);
	ERROR_CHECK_NEG(r, "Error getting jsonrpc request.");

	r = jsonrpc_send_request(&response, json_request);
	ERROR_CHECK_NEG(r, "Could not get json-rpc response.");

	r = jsonrpc_parse_response(&jobj, (char *)response);
	ERROR_CHECK_NEG(r, "Could not parse json-rpc response.");

	r = jsonrpc_has_error(jobj);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");
	ERROR_CHECK_FALSE(r, "JSON-RPC response conained an error flag.");

	r = json_get_key_obj(&result, jobj, JSONRPC_RESULT_KEY);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");

	if (!cJSON_IsNumber(result))
	{
		error_log("Response JSON did not contain result in expected number format.");
		return -1;
	}

	*count = result->valueint;

	json_free(jobj);

	return 1;
}

int jsonrpc_get_blockhash(char *hash, int blocknum)
{
	int r;
	char json_request[BUFSIZ];
	unsigned char *response = NULL;
	char param_str[BUFSIZ];
	cJSON *jobj = NULL;
	cJSON *result;

	assert(hash);

	memset(json_request, 0, BUFSIZ);
	memset(param_str, 0, BUFSIZ);

	sprintf(param_str, "%i", blocknum);

	r = jsonrpc_build_request(json_request, BUFSIZ - 1, "getblockhash", param_str);
	ERROR_CHECK_NEG(r, "Error getting jsonrpc request.");

	r = jsonrpc_send_request(&response, json_request);
	ERROR_CHECK_NEG(r, "Could not get json-rpc response.");

	r = jsonrpc_parse_response(&jobj, (char *)response);
	ERROR_CHECK_NEG(r, "Could not parse json-rpc response.");

	r = jsonrpc_has_error(jobj);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");
	ERROR_CHECK_FALSE(r, "JSON-RPC response conained an error flag.");

	r = json_get_key_obj(&result, jobj, JSONRPC_RESULT_KEY);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");

	if (!cJSON_IsString(result))
	{
		error_log("Response JSON did not contain result in expected string format.");
		return -1;
	}

	strcpy(hash, result->valuestring);

	json_free(jobj);
	free(response);


	return 1;
}

int jsonrpc_get_block(char **block, char *blockhash)
{
	int r;
	int verbosity = 0;
	char json_request[BUFSIZ];
	char param_str[BUFSIZ];
	unsigned char *response = NULL;
	cJSON *jobj = NULL;
	cJSON *result;

	assert(blockhash);

	memset(json_request, 0, BUFSIZ);
	memset(param_str, 0, BUFSIZ);

	sprintf(param_str, "\"%s\", %i", blockhash, verbosity);

	r = jsonrpc_build_request(json_request, BUFSIZ - 1, "getblock", param_str);
	ERROR_CHECK_NEG(r, "Error getting jsonrpc request.");

	r = jsonrpc_send_request(&response, json_request);
	ERROR_CHECK_NEG(r, "Could not get json-rpc response.");

	r = jsonrpc_parse_response(&jobj, (char *)response);
	ERROR_CHECK_NEG(r, "Could not parse json-rpc response.");

	r = jsonrpc_has_error(jobj);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");
	ERROR_CHECK_FALSE(r, "JSON-RPC response conained an error flag.");

	r = json_get_key_obj(&result, jobj, JSONRPC_RESULT_KEY);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");

	if (!cJSON_IsString(result))
	{
		error_log("Response JSON did not contain result in expected object format.");
		return -1;
	}

	(*block) = malloc(strlen(result->valuestring) + 1);
	ERROR_CHECK_NULL((*block), "Memory allocation error.");

	strcpy((*block), result->valuestring);

	json_free(jobj);
	free(response);

	return 1;
}

int jsonrpc_get_rawtransaction(char **tx_raw, char *tx_hash)
{
	int r;
	char json_request[BUFSIZ];
	char param_str[BUFSIZ];
	unsigned char *response = NULL;
	cJSON *jobj = NULL;
	cJSON *result;

	assert(tx_hash);

	memset(json_request, 0, BUFSIZ);
	memset(param_str, 0, BUFSIZ);

	sprintf(param_str, "\"%s\"", tx_hash);

	r = jsonrpc_build_request(json_request, BUFSIZ - 1, "getrawtransaction", param_str);
	ERROR_CHECK_NEG(r, "Error getting jsonrpc request.");

	r = jsonrpc_send_request(&response, json_request);
	ERROR_CHECK_NEG(r, "Could not get json-rpc response.");

	r = jsonrpc_parse_response(&jobj, (char *)response);
	ERROR_CHECK_NEG(r, "Could not parse json-rpc response.");

	r = jsonrpc_has_error(jobj);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");
	ERROR_CHECK_FALSE(r, "JSON-RPC response conained an error flag.");

	r = json_get_key_obj(&result, jobj, JSONRPC_RESULT_KEY);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");

	if (!cJSON_IsString(result))
	{
		error_log("Response JSON did not contain result in expected string format.");
		return -1;
	}

	(*tx_raw) = malloc(strlen(result->valuestring) + 1);
	ERROR_CHECK_NULL((*tx_raw), "Memory allocation error.");

	strcpy((*tx_raw), result->valuestring);

	json_free(jobj);
	free(response);

	return 1;
}

int jsonrpc_build_request(char *output, int max_len, char *method, char *params)
{
	char data[BUFSIZ];
	char tmp[BUFSIZ];

	if (params == NULL)
	{
		params = "";
	}

	assert(output);
	assert(max_len > 0);

	memset(data, 0, BUFSIZ);
	snprintf(data, BUFSIZ, JSONRPC_DATA, method, params);

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_START);
	strncat(output, tmp, max_len - strlen(output));

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_HOST, hostname);
	strncat(output, tmp, max_len - strlen(output));

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_AUTH, auth_str);
	strncat(output, tmp, max_len - strlen(output));

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_CONTYPE);
	strncat(output, tmp, max_len - strlen(output));

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_CONLEN, strlen(data));
	strncat(output, tmp, max_len - strlen(output));

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_DATA_START);
	strncat(output, tmp, max_len - strlen(output));

	strncat(output, data, max_len - strlen(output));

	memset(tmp, 0, BUFSIZ);
	snprintf(tmp, BUFSIZ, JSONRPC_DATA_END);
	strncat(output, tmp, max_len - strlen(output));

	return 1;
}

int jsonrpc_send_request(unsigned char **response, char *json_request)
{
	int r;
	Node node;

	assert(hostname);
	assert(service);
	assert(auth_str);
	assert(json_request);

	r = node_new(&node, hostname, service);
	ERROR_CHECK_NEG(r, "Could not get new node object.");

	r = node_connect(node);
	ERROR_CHECK_NEG(r, "Could not connect to host.");

	r = node_write(node, (unsigned char*)json_request, strlen(json_request));
	ERROR_CHECK_NEG(r, "Could not send message to host.");

	r = node_read(node, response);
	ERROR_CHECK_NEG(r, "Could not read message from host.");
	ERROR_CHECK_FALSE(r, "No data before timeout.");

	node_disconnect(node);
	node_destroy(node);

	return r;
}

int jsonrpc_parse_response(cJSON **jobj, char *response)
{
	assert(response);

	response = index(response, '{');
	ERROR_CHECK_NULL(response, "Response did not contain JSON.");

	(*jobj) = cJSON_ParseWithOpts(response, NULL, 1);
	ERROR_CHECK_NULL((*jobj), "Response contained invalid JSON.");

	if (!cJSON_IsObject((*jobj)))
	{
		error_log("Response JSON not formatted as an object.");
		return -1;
	}

	return 1;
}

int jsonrpc_has_error(cJSON *jobj)
{
	int r;
	cJSON *error = NULL;
	char *error_str = NULL;

	r = json_get_key_obj(&error, jobj, JSONRPC_ERROR_KEY);
	ERROR_CHECK_NEG(r, "Could not check json-rpc error status.");

	if (cJSON_IsNull(error))
	{
		return 1;
	}

	r = json_to_string(&error_str, error);
	if (r < 0)
	{
		error_log("Could not print error info.");
	}
	else
	{
		error_log("Error msg: \n%s", error_str);
		free(error_str);
	}

	return 0;
}