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

#define BLOCK_STRING "\xe2\x96\x88\xe2\x96\x88"

int qrcode_from_str(char *output, char *input)
{
    int x, y;
    int size;
    int border = 0;
    bool ok;
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;

    assert(input);
    assert(output);

    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

    ok = qrcodegen_encodeText(input, tempBuffer, qrcode, errCorLvl, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    ERROR_CHECK_FALSE(ok, "Could not encode QR code.");

    size = qrcodegen_getSize(qrcode);
    for (y = -border; y < size + border; y++)
    {
        for (x = -border; x < size + border; x++)
        {
            strcat(output, (qrcodegen_getModule(qrcode, x, y) ? BLOCK_STRING : "  "));
        }
        strcat(output, "\n");
    }

    return 1;
}