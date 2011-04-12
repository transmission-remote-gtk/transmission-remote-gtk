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

#ifndef REQUESTS_H_
#define REQUESTS_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

JsonNode *generic_request(gchar * method, JsonArray * array);

JsonNode *session_set(void);
JsonNode *session_get(void);
JsonNode *torrent_get(gboolean recent);
JsonNode *torrent_set(JsonArray * array);
JsonNode *torrent_pause(JsonArray * array);
JsonNode *torrent_start(JsonArray * array);
JsonNode *torrent_verify(JsonArray * array);
JsonNode *torrent_reannounce(JsonArray * array);
JsonNode *torrent_remove(JsonArray * array, int removeData);
JsonNode *torrent_add(gchar * filename, gboolean paused);
JsonNode *torrent_add_url(const gchar * url, gboolean paused);
JsonNode *torrent_set_location(JsonArray * array, gchar * location,
                               gboolean move);
JsonNode *blocklist_update(void);
JsonNode *port_test(void);
JsonNode *session_stats(void);

#endif                          /* REQUESTS_H_ */
