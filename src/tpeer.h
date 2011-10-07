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

#ifndef TPEER_H_
#define TPEER_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

const gchar *peer_get_address(JsonObject * p);
const gchar *peer_get_client_name(JsonObject * p);
gboolean peer_get_is_encrypted(JsonObject * p);
gdouble peer_get_progress(JsonObject * p);
const gchar *peer_get_flagstr(JsonObject * p);
gint64 peer_get_rate_to_client(JsonObject * p);
gint64 peer_get_rate_to_peer(JsonObject * p);
gboolean peer_get_is_uploading_to(JsonObject * p);
gboolean peer_get_is_downloading_from(JsonObject * p);

gint64 peerfrom_get_pex(JsonObject *pf);
gint64 peerfrom_get_dht(JsonObject *pf);
gint64 peerfrom_get_trackers(JsonObject *pf);
gint64 peerfrom_get_ltep(JsonObject *pf);
gint64 peerfrom_get_resume(JsonObject *pf);
gint64 peerfrom_get_incoming(JsonObject *pf);
gint64 peerfrom_get_lpd(JsonObject *pf);

#endif                          /* TPEER_H_ */
