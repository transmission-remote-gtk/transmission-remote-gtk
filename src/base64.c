/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011  Alan Fitton

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>

#include "base64.h"

static const char cb64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void encodeblock(unsigned char in[3], unsigned char out[4], int len)
{
    out[0] = cb64[in[0] >> 2];
    out[1] = cb64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
    out[2] =
	(unsigned char) (len >
			 1 ? cb64[((in[1] & 0x0f) << 2) |
				  ((in[2] & 0xc0) >> 6)] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[in[2] & 0x3f] : '=');
}

char *base64encode(char *filename)
{
    FILE *infile = fopen(filename, "rb");
    unsigned char in[3], out[4];
    int i, len, j = 0;
    char *output = NULL;
    while (!feof(infile)) {
	len = 0;
	for (i = 0; i < 3; i++) {
	    in[i] = (unsigned char) getc(infile);
	    if (!feof(infile)) {
		len++;
	    } else {
		in[i] = 0;
	    }
	}
	if (len) {
	    output = (char *) realloc(output, j + 5);
	    encodeblock(in, out, len);
	    for (i = 0; i < 4; i++) {
		output[j++] = out[i];
	    }
	}
    }
    output[j] = '\0';
    fclose(infile);
    return output;
}
