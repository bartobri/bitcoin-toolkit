/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "error.h"
#include "QRCodeGen/qrcodegen.h"

#define BLOCK_UL        "\xe2\x96\x80 "
#define BLOCK_UR        " \xe2\x96\x80"
#define BLOCK_LL        "\xe2\x96\x84 "
#define BLOCK_LR        " \xe2\x96\x84"
#define BLOCK_UL_LL     "\xe2\x96\x88 "
#define BLOCK_UR_LR     " \xe2\x96\x88"
#define BLOCK_UL_UR     "\xe2\x96\x80\xe2\x96\x80"
#define BLOCK_LL_LR     "\xe2\x96\x84\xe2\x96\x84"
#define BLOCK_UL_LR     "\xe2\x96\x80\xe2\x96\x84"
#define BLOCK_UR_LL     "\xe2\x96\x84\xe2\x96\x80"
#define BLOCK_UL_LL_LR  "\xe2\x96\x88\xe2\x96\x84"
#define BLOCK_UL_UR_LL  "\xe2\x96\x88\xe2\x96\x80"
#define BLOCK_UL_UR_LR  "\xe2\x96\x80\xe2\x96\x88"
#define BLOCK_UR_LL_LR  "\xe2\x96\x84\xe2\x96\x88"
#define BLOCK_FULL      "\xe2\x96\x88\xe2\x96\x88"
#define BLOCK_EMPTY     "  "

int qrcode_from_str(char *output, char *input)
{
	int x, y;
	int size;
	int border = 0;
	bool ok;
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;

	int ul, ur, ll, lr;
	char *block_string = NULL;

	assert(input);
	assert(output);

	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	ok = qrcodegen_encodeText(input, tempBuffer, qrcode, errCorLvl, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	ERROR_CHECK_FALSE(ok, "Could not encode QR code.");

	size = qrcodegen_getSize(qrcode);
	for (y = -border; y < size + border; y += 2)
	{
		for (x = -border; x < size + border; x += 2)
		{
			ul = qrcodegen_getModule(qrcode, x, y);
			ur = qrcodegen_getModule(qrcode, x+1, y);
			ll = qrcodegen_getModule(qrcode, x, y+1);
			lr = qrcodegen_getModule(qrcode, x+1, y+1);

			if (ul && !ur && !ll && !lr)
				block_string = BLOCK_UL;
			else if (!ul && ur && !ll && !lr)
				block_string = BLOCK_UR;
			else if (!ul && !ur && ll && !lr)
				block_string = BLOCK_LL;
			else if (!ul && !ur && !ll && lr)
				block_string = BLOCK_LR;
			else if (ul && ur && !ll && !lr)
				block_string = BLOCK_UL_UR;
			else if (!ul && !ur && ll && lr)
				block_string = BLOCK_LL_LR;
			else if (ul && !ur && ll && !lr)
				block_string = BLOCK_UL_LL;
			else if (!ul && ur && !ll && lr)
				block_string = BLOCK_UR_LR;
			else if (ul && !ur && !ll && lr)
				block_string = BLOCK_UL_LR;
			else if (!ul && ur && ll && !lr)
				block_string = BLOCK_UR_LL;
			else if (ul && ur && ll && !lr)
				block_string = BLOCK_UL_UR_LL;
			else if (ul && ur && !ll && lr)
				block_string = BLOCK_UL_UR_LR;
			else if (ul && !ur && ll && lr)
				block_string = BLOCK_UL_LL_LR;
			else if (!ul && ur && ll && lr)
				block_string = BLOCK_UR_LL_LR;
			else if (ul && ur && ll && lr)
				block_string = BLOCK_FULL;
			else
				block_string = BLOCK_EMPTY;

			strcat(output, block_string);

			// Leaving here in case I implement an option to print a bigger size
			//strcat(output, (qrcodegen_getModule(qrcode, x, y) ? "\xe2\x96\x88\xe2\x96\x88" : " "));
		}
		strcat(output, "\n");
	}

	return 1;
}