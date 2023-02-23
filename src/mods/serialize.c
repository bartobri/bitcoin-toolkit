/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "serialize.h"

unsigned char *serialize_uint8(unsigned char *dest, uint8_t src, int endian)
{
	assert(dest);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);
	
	*dest++ = (unsigned char)src;
	
	return dest;
}

unsigned char *serialize_uint16(unsigned char *dest, uint16_t src, int endian)
{
	assert(dest);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);

	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		*dest++ = (unsigned char)(src & 0x00FF);
		*dest++ = (unsigned char)((src & 0xFF00) >> 8);
	}
	else
	{
		*dest++ = (unsigned char)((src & 0xFF00) >> 8);
		*dest++ = (unsigned char)(src & 0x00FF);
	}
	
	return dest;
}

unsigned char *serialize_uint32(unsigned char *dest, uint32_t src, int endian)
{
	assert(dest);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);

	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		*dest++ = (unsigned char)(src & 0x000000FF);
		*dest++ = (unsigned char)((src & 0x0000FF00) >> 8);
		*dest++ = (unsigned char)((src & 0x00FF0000) >> 16);
		*dest++ = (unsigned char)((src & 0xFF000000) >> 24);
	}
	else
	{
		*dest++ = (unsigned char)((src & 0xFF000000) >> 24);
		*dest++ = (unsigned char)((src & 0x00FF0000) >> 16);
		*dest++ = (unsigned char)((src & 0x0000FF00) >> 8);
		*dest++ = (unsigned char)(src & 0x000000FF);
	}
	
	return dest;
}

unsigned char *serialize_uint64(unsigned char *dest, uint64_t src, int endian)
{
	assert(dest);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);

	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		*dest++ = (unsigned char)(src & 0x00000000000000FF);
		*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
		*dest++ = (unsigned char)((src & 0x0000000000FF0000) >> 16);
		*dest++ = (unsigned char)((src & 0x00000000FF000000) >> 24);
		*dest++ = (unsigned char)((src & 0x000000FF00000000) >> 32);
		*dest++ = (unsigned char)((src & 0x0000FF0000000000) >> 40);
		*dest++ = (unsigned char)((src & 0x00FF000000000000) >> 48);
		*dest++ = (unsigned char)((src & 0xFF00000000000000) >> 56);
	}
	else
	{
		*dest++ = (unsigned char)((src & 0xFF00000000000000) >> 56);
		*dest++ = (unsigned char)((src & 0x00FF000000000000) >> 48);
		*dest++ = (unsigned char)((src & 0x0000FF0000000000) >> 40);
		*dest++ = (unsigned char)((src & 0x000000FF00000000) >> 32);
		*dest++ = (unsigned char)((src & 0x00000000FF000000) >> 24);
		*dest++ = (unsigned char)((src & 0x0000000000FF0000) >> 16);
		*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
		*dest++ = (unsigned char)(src & 0x00000000000000FF);
	}
	
	return dest;
}

unsigned char *serialize_uchar(unsigned char *dest, unsigned char *src, int len)
{
	assert(dest);
	assert(src);
	
	memcpy(dest, src, len);
	dest += len;
	
	return dest;
}

unsigned char *serialize_char(unsigned char *dest, char *src, int len)
{
	assert(dest);
	assert(src);
	
	memcpy(dest, src, len);
	dest += len;
	
	return dest;
}

unsigned char *serialize_compuint(unsigned char *dest, uint64_t src, int endian)
{
	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);

	if (src <= 0xfc)
	{
		*dest++ = (unsigned char)src;
	}
	else if (src <= 0xffff)
	{
		*dest++ = 0xfd;
		if (endian == SERIALIZE_ENDIAN_LIT)
		{
			*dest++ = (unsigned char)(src & 0x00000000000000FF);
			*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
		}
		else
		{
			*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
			*dest++ = (unsigned char)(src & 0x00000000000000FF);
		}
	}
	else if (src <= 0xffffffff)
	{
		*dest++ = 0xfe;
		if (endian == SERIALIZE_ENDIAN_LIT)
		{
			*dest++ = (unsigned char)(src & 0x00000000000000FF);
			*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
			*dest++ = (unsigned char)((src & 0x0000000000FF0000) >> 16);
			*dest++ = (unsigned char)((src & 0x00000000FF000000) >> 24);
		}
		else
		{
			*dest++ = (unsigned char)((src & 0x00000000FF000000) >> 24);
			*dest++ = (unsigned char)((src & 0x0000000000FF0000) >> 16);
			*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
			*dest++ = (unsigned char)(src & 0x00000000000000FF);
		}
	}
	else if (src <= 0xffffffffffffffff)
	{
		*dest++ = 0xff;
		if (endian == SERIALIZE_ENDIAN_LIT)
		{
			*dest++ = (unsigned char)(src & 0x00000000000000FF);
			*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
			*dest++ = (unsigned char)((src & 0x0000000000FF0000) >> 16);
			*dest++ = (unsigned char)((src & 0x00000000FF000000) >> 24);
			*dest++ = (unsigned char)((src & 0x000000FF00000000) >> 32);
			*dest++ = (unsigned char)((src & 0x0000FF0000000000) >> 40);
			*dest++ = (unsigned char)((src & 0x00FF000000000000) >> 48);
			*dest++ = (unsigned char)((src & 0xFF00000000000000) >> 56);
		}
		else
		{
			*dest++ = (unsigned char)((src & 0xFF00000000000000) >> 56);
			*dest++ = (unsigned char)((src & 0x00FF000000000000) >> 48);
			*dest++ = (unsigned char)((src & 0x0000FF0000000000) >> 40);
			*dest++ = (unsigned char)((src & 0x000000FF00000000) >> 32);
			*dest++ = (unsigned char)((src & 0x00000000FF000000) >> 24);
			*dest++ = (unsigned char)((src & 0x0000000000FF0000) >> 16);
			*dest++ = (unsigned char)((src & 0x000000000000FF00) >> 8);
			*dest++ = (unsigned char)(src & 0x00000000000000FF);
		}
	}
	
	return dest;
}

unsigned char *serialize_varint(unsigned char *dest, uint64_t src)
{
	int i = 0;
	unsigned char tmp[10];

	assert(dest);

	while (1)
	{
		tmp[i] = (src & 0x7F) | (i ? 0x80 : 0x00);
		if (src <= 0x7F)
		{
			break;
		}
		src = (src >> 7) - 1;
		i++;
	}

	do {
		*dest++ = tmp[i];
	} while (i--);

	return dest;
}

unsigned char *deserialize_uint8(uint8_t *dest, unsigned char *src, int endian)
{
	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);
	
	*dest = 0;
	
	// Endian doesn't matter for 8 bit ints

	*dest += ((uint8_t)*src++);
	
	return src;
}

unsigned char *deserialize_uint16(uint16_t *dest, unsigned char *src, int endian)
{
	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);
	
	*dest = 0;
	
	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		*dest += ((uint16_t)*src++);
		*dest += (((uint16_t)*src++) << 8);
	}
	else
	{
		*dest += (((uint16_t)*src++) << 8);
		*dest += ((uint16_t)*src++);
	}
	
	return src;
}

unsigned char *deserialize_uint32(uint32_t *dest, unsigned char *src, int endian)
{
	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);
	
	*dest = 0;
	
	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		*dest += *src++;
		*dest += (*src++ << 8);
		*dest += (*src++ << 16);
		*dest += (*src++ << 24);
	}
	else
	{
		*dest += (*src++ << 24);
		*dest += (*src++ << 16);
		*dest += (*src++ << 8);
		*dest += *src++;
	}
	
	return src;
}

unsigned char *deserialize_uint64(uint64_t *dest, unsigned char *src, int endian)
{
	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);
	
	*dest = 0;
	
	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		*dest += ((uint64_t)*src++);
		*dest += (((uint64_t)*src++) << 8);
		*dest += (((uint64_t)*src++) << 16);
		*dest += (((uint64_t)*src++) << 24);
		*dest += (((uint64_t)*src++) << 32);
		*dest += (((uint64_t)*src++) << 40);
		*dest += (((uint64_t)*src++) << 48);
		*dest += (((uint64_t)*src++) << 56);
	}
	else
	{
		*dest += (((uint64_t)*src++) << 56);
		*dest += (((uint64_t)*src++) << 48);
		*dest += (((uint64_t)*src++) << 40);
		*dest += (((uint64_t)*src++) << 32);
		*dest += (((uint64_t)*src++) << 24);
		*dest += (((uint64_t)*src++) << 16);
		*dest += (((uint64_t)*src++) << 8);
		*dest += ((uint64_t)*src++);
	}
	
	return src;
}

unsigned char *deserialize_uchar(unsigned char *dest, unsigned char *src, int len, int endian)
{
	int i;

	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);

	if (endian == SERIALIZE_ENDIAN_LIT)
	{
		for (i = 0; i < len; i++)
		{
			dest[i] = src[len - 1 - i];
		}
	}
	else
	{
		memcpy(dest, src, len);
	}

	src += len;
	
	return src;
}

unsigned char *deserialize_char(char *dest, unsigned char *src, int len)
{
	assert(dest);
	assert(src);
	
	memcpy(dest, src, len);
	src += len;
	
	return src;
}

unsigned char *deserialize_compuint(uint64_t *dest, unsigned char *src, int endian)
{
	assert(dest);
	assert(src);
	assert(endian == SERIALIZE_ENDIAN_BIG || endian == SERIALIZE_ENDIAN_LIT);
	
	*dest = 0;

	if (*src <= 0xfc)
	{
		*dest += ((uint64_t)*src++);
	}
	else if (*src <= 0xfd)
	{
		src++;
		if (endian == SERIALIZE_ENDIAN_LIT)
		{
			*dest += ((uint64_t)*src++);
			*dest += (((uint64_t)*src++) << 8);
		}
		else
		{
			*dest += (((uint64_t)*src++) << 8);
			*dest += ((uint64_t)*src++);
		}
	}
	else if (*src <= 0xfe)
	{
		src++;
		if (endian == SERIALIZE_ENDIAN_LIT)
		{
			*dest += ((uint64_t)*src++);
			*dest += (((uint64_t)*src++) << 8);
			*dest += (((uint64_t)*src++) << 16);
			*dest += (((uint64_t)*src++) << 24);
		}
		else
		{
			*dest += (((uint64_t)*src++) << 24);
			*dest += (((uint64_t)*src++) << 16);
			*dest += (((uint64_t)*src++) << 8);
			*dest += ((uint64_t)*src++);
		}
	}
	else
	{
		src++;
		if (endian == SERIALIZE_ENDIAN_LIT)
		{
			*dest += ((uint64_t)*src++);
			*dest += (((uint64_t)*src++) << 8);
			*dest += (((uint64_t)*src++) << 16);
			*dest += (((uint64_t)*src++) << 24);
			*dest += (((uint64_t)*src++) << 32);
			*dest += (((uint64_t)*src++) << 40);
			*dest += (((uint64_t)*src++) << 48);
			*dest += (((uint64_t)*src++) << 56);
		}
		else
		{
			*dest += (((uint64_t)*src++) << 56);
			*dest += (((uint64_t)*src++) << 48);
			*dest += (((uint64_t)*src++) << 40);
			*dest += (((uint64_t)*src++) << 32);
			*dest += (((uint64_t)*src++) << 24);
			*dest += (((uint64_t)*src++) << 16);
			*dest += (((uint64_t)*src++) << 8);
			*dest += ((uint64_t)*src++);
		}
	}
	
	return src;
}

unsigned char *deserialize_varint(uint64_t *dest, unsigned char *src)
{
	assert(dest);
	assert(src);

	*dest = 0;

	while (1)
	{
		*dest = (*dest << 7) | (uint64_t)(*src & 0x7F);
		if (*src >= 0x80)
		{
			*dest += 1;
		}
		else
		{
			break;
		}
		src++;
	}

	src++;

	return src;
}