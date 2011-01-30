/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2010  Alan Fitton

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

#define TPEER_ADDRESS           "address"
#define TPEER_CLIENT_NAME       "client_name"
#define TPEER_PROGRESS          "progress"
#define TPEER_RATE_TO_CLIENT    "rateToClient"
#define TPEER_RATE_TO_PEER      "rateToPeer"
#define TPEER_IS_ENCRYPTED      "isEncrypted"
#define TPEER_IS_DOWNLOADING_FROM "isDownloadingFrom"
#define TPEER_IS_UPLOADING_TO "isUploadingTo"

#define TPEER_FLAGSTR            "flagStr"

const gchar *peer_get_address(JsonObject * p);
const gchar *peer_get_client_name(JsonObject * p);
gboolean peer_get_is_encrypted(JsonObject * p);
gdouble peer_get_progress(JsonObject * p);
const gchar *peer_get_flagstr(JsonObject * p);
gint64 peer_get_rate_to_client(JsonObject * p);
gint64 peer_get_rate_to_peer(JsonObject * p);
gboolean peer_get_is_uploading_to(JsonObject * p);
gboolean peer_get_is_downloading_from(JsonObject * p);

#endif				/* TPEER_H_ */
