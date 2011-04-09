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

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <glib.h>

#include "base64.h"

#define my_print_errno(x) printf("%s: error (%d) %s\n", __func__, errno, x);

char *base64encode(char *filename)
{
    gint fd = open(filename, O_RDONLY);
    struct stat sb;
    void *addr;
    gchar *b64out = NULL;

    if (fd < 0) {
        my_print_errno("opening file");
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        my_print_errno("on fstat");
        close(fd);
        return NULL;
    }

    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        my_print_errno("on mmap");
        close(fd);
        return NULL;
    }

    b64out = g_base64_encode((guchar*) addr, sb.st_size);
    munmap(addr, sb.st_size);
    close(fd);

    return b64out;
}
