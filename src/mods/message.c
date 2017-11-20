#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "crypto.h"
#include "message.h"
#include "mem.h"
#include "assert.h"
#include "messages/version.h"

#define MESSAGE_MAINNET        0xD9B4BEF9
#define MESSAGE_TESTNET        0x0709110B
#define MESSAGE_COMMAND_MAXLEN 12
#define MESSAGE_PAYLOAD_MAXLEN 1024

struct Message {
	uint32_t       magic;
	char           command[MESSAGE_COMMAND_MAXLEN];
	uint32_t       length;
	uint32_t       checksum;
	unsigned char *payload;
};

Message message_new(const char *c) {
	Message m;
	unsigned char *sha1, *sha2;

	assert(c);

	NEW0(m);

	m->magic = MESSAGE_MAINNET;
	strncpy(m->command, c, MESSAGE_COMMAND_MAXLEN);

	if (strcmp(m->command, "version") == 0) {
		m->length = (uint32_t)version_serialize(version_new(), &(m->payload));
	} else {
		m->length = 0;
		m->payload = NULL;
	}
	
	// Getting checksum of payload
	sha1 = crypto_get_sha256(m->payload, m->length);
	sha2 = crypto_get_sha256(sha1, 32);
	m->checksum <<= 8;
	m->checksum += sha2[0];
	m->checksum <<= 8;
	m->checksum += sha2[1];
	m->checksum <<= 8;
	m->checksum += sha2[2];
	m->checksum <<= 8;
	m->checksum += sha2[3];	
	FREE(sha1);
	FREE(sha2);

	return m;
}

size_t message_serialize(Message m, unsigned char **s) {
	size_t i, len = 0;
	unsigned char *temp;
	
	temp = ALLOC(sizeof(struct Message) + MESSAGE_PAYLOAD_MAXLEN);
	
	// Serializing Magic (little endian)
	temp[len++] = (unsigned char)(m->magic & 0x000000FF);
	temp[len++] = (unsigned char)((m->magic & 0x0000FF00) >> 8);
	temp[len++] = (unsigned char)((m->magic & 0x00FF0000) >> 16);
	temp[len++] = (unsigned char)((m->magic & 0xFF000000) >> 24);
	
	// Serializing command
	for (i = 0; i < MESSAGE_COMMAND_MAXLEN; ++i) {
		if (m->command[i])
			temp[i+len] = m->command[i];
		else
			temp[i+len] = 0x00;
	}
	len += MESSAGE_COMMAND_MAXLEN;
	
	// Serializing Length (little endian)
	temp[len++] = (unsigned char)(m->length & 0x000000FF);
	temp[len++] = (unsigned char)((m->length & 0x0000FF00) >> 8);
	temp[len++] = (unsigned char)((m->length & 0x00FF0000) >> 16);
	temp[len++] = (unsigned char)((m->length & 0xFF000000) >> 24);
	
	// Serializing checksum (big endian)
	temp[len++] = (unsigned char)((m->checksum & 0xFF000000) >> 24);
	temp[len++] = (unsigned char)((m->checksum & 0x00FF0000) >> 16);
	temp[len++] = (unsigned char)((m->checksum & 0x0000FF00) >> 8);
	temp[len++] = (unsigned char)(m->checksum & 0x000000FF);
	
	// Append payload data
	memcpy(temp + len, m->payload, m->length);
	len += m->length;
	
	*s = temp;
	
	return len;
}

Message message_from_raw(unsigned char *data, int l) {
	size_t i;
	Message m;

	assert(data);
	assert(l);

	NEW0(m);
	
	// De-Serializing Magic (little endian)
	m->magic += *data++;
	m->magic += (*data++ << 8);
	m->magic += (*data++ << 16);
	m->magic += (*data++ << 24);
	
	// De-Serializing command
	for (i = 0; i < MESSAGE_COMMAND_MAXLEN; ++i) {
		if (data[i])
			m->command[i] = data[i];
		else
			m->command[i] = 0x00;
	}
	data += MESSAGE_COMMAND_MAXLEN;
	
	// De-Serializing Length (little endian)
	m->length += *data++;
	m->length += (*data++ << 8);
	m->length += (*data++ << 16);
	m->length += (*data++ << 24);
	
	// De-Serializing checksum (big endian)
	m->checksum += (*data++ << 24);
	m->checksum += (*data++ << 16);
	m->checksum += (*data++ << 8);
	m->checksum += *data++;
	
	// Getting payload
	m->payload = ALLOC(m->length);
	for (i = 0; i < m->length; ++i) {
		m->payload[i] = *data++;
	}
	
	return m;
}
