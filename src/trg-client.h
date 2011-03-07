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

#ifndef TRG_CLIENT_H_
#define TRG_CLIENT_H_

#define TRG_GCONF_SCHEMA_ERROR -1
#define TRG_NO_HOSTNAME_SET -2

#include <json-glib/json-glib.h>
#include <gconf/gconf-client.h>

#include "session-get.h"

typedef struct {
    char *session_id;
    gint failCount;
    gint interval;
    gint64 updateSerial;
    JsonObject *session;
    gboolean ssl;
    float version;
    char *url;
    char *username;
    char *password;
    char *proxy;
    JsonArray *torrents;
    GConfClient *gconf;
    GMutex *updateMutex;
} trg_client;

trg_client *trg_init_client();
int trg_client_populate_with_settings(trg_client * tc,
                                      GConfClient * gconf);
void trg_client_set_session(trg_client * tc, JsonObject * session);
gboolean trg_client_supports_tracker_edit(trg_client * tc);

#endif                          /* TRG_CLIENT_H_ */
