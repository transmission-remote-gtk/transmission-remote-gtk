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
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "base64.h"

gchar *base64encode(const gchar *filename)
{
    GError *error = NULL;
    GMappedFile *mf =  g_mapped_file_new(filename, FALSE, &error);
    gchar *b64out = NULL;

    if (error)
    {
        g_error(error->message);
        g_error_free(error);
    } else {
        b64out = g_base64_encode((guchar*)g_mapped_file_get_contents(mf), g_mapped_file_get_length(mf));
    }

    g_mapped_file_unref(mf);

    return b64out;
}
