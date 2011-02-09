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

#include <glib-object.h>
#include <gconf/gconf-client.h>

#include "trg-client.h"
#include "trg-preferences.h"

trg_client *trg_init_client()
{
    trg_client *client;

    client = g_new0(trg_client, 1);
    client->gconf = gconf_client_get_default();
    client->updateMutex = g_mutex_new();

    return client;
}

#define check_for_error(error) if (error) { g_error_free(error); return FALSE; }

gboolean trg_client_populate_with_settings(trg_client * tc,
					   GConfClient * gconf)
{
    gint port;
    gchar *host;
    GError *error = NULL;

    g_free(tc->url);
    tc->url = NULL;

    g_free(tc->username);
    tc->username = NULL;

    g_free(tc->password);
    tc->password = NULL;

    port = gconf_client_get_int(gconf, TRG_GCONF_KEY_PORT, &error);
    check_for_error(error);

    host = gconf_client_get_string(gconf, TRG_GCONF_KEY_HOSTNAME, &error);
    check_for_error(error);

	tc->url = g_strdup_printf("http://%s:%d/transmission/rpc", host, port);
	g_free(host);

    tc->username = gconf_client_get_string(gconf, TRG_GCONF_KEY_USERNAME, &error);
    check_for_error(error);

    tc->password = gconf_client_get_string(gconf, TRG_GCONF_KEY_PASSWORD, &error);
    check_for_error(error);

    return TRUE;
}
