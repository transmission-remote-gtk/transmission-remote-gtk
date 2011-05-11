/*
 * transmission-remote-gtk - Transmission RPC client for GTK
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

#include <glib.h>
#include <gconf/gconf-client.h>

#include "util.h"
#include "trg-preferences.h"

gboolean pref_get_add_options_dialog(GConfClient * gcc)
{
    return gconf_client_get_bool_or_true(gcc,
                                         TRG_GCONF_KEY_ADD_OPTIONS_DIALOG);
}

gboolean pref_get_start_paused(GConfClient * gcc)
{
    return gconf_client_get_bool(gcc, TRG_GCONF_KEY_START_PAUSED, NULL);
}
