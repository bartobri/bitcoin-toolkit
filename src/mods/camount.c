/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

/*
 * This code was translated from:
 * https://github.com/bitcoin/bitcoin/blob/v0.13.2/src/compressor.cpp#L133#L185
 */

#include <stdint.h>
#include <assert.h>

void camount_compress(uint64_t *dest, uint64_t src)
{
    int d, e = 0;

    if (src == 0)
    {
        *dest = 0;
    }
    else
    {
        while (((src % 10) == 0) && e < 9)
        {
            src /= 10;
            e++;
        }

        if (e < 9)
        {
            d = (src % 10);

            assert(d >= 1 && d <= 9);

            src /= 10;
            *dest = 1 + (src * 9 + d - 1) * 10 + e;
        }
        else
        {
            *dest = 1 + (src - 1) * 10 + 9;
        }
    }
}

void camount_decompress(uint64_t *dest, uint64_t src)
{
    int e, d;
    uint64_t n = 0;

    if (src == 0)
    {
        *dest = 0;
    }
    else
    {
        src--;

        e = src % 10;
        src /= 10;

        if (e < 9)
        {
            d = (src % 9) + 1;
            src /= 9;
            n = src * 10 + d;
        }
        else
        {
            n = src + 1;
        }

        while (e)
        {
            n *= 10;
            e--;
        }

        *dest = n;
    }
}