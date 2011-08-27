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
#include <json-glib/json-glib.h>

#include "tpeer.h"
#include "protocol-constants.h"

const gchar *peer_get_address(JsonObject * p) {
    return json_object_get_string_member(p, TPEER_ADDRESS);
}

const gchar *peer_get_flagstr(JsonObject * p) {
    return json_object_get_string_member(p, TPEER_FLAGSTR);
}

const gchar *peer_get_client_name(JsonObject * p) {
    return json_object_get_string_member(p, TPEER_CLIENT_NAME);
}

gboolean peer_get_is_encrypted(JsonObject * p) {
    return json_object_get_boolean_member(p, TPEER_IS_ENCRYPTED);
}

gboolean peer_get_is_uploading_to(JsonObject * p) {
    return json_object_get_boolean_member(p, TPEER_IS_UPLOADING_TO);
}

gboolean peer_get_is_downloading_from(JsonObject * p) {
    return json_object_get_boolean_member(p, TPEER_IS_DOWNLOADING_FROM);
}

gdouble peer_get_progress(JsonObject * p) {
    return json_object_get_double_member(p, TPEER_PROGRESS) * 100.0;
}

gint64 peer_get_rate_to_client(JsonObject * p) {
    return json_node_get_int(json_object_get_member(p, TPEER_RATE_TO_CLIENT));
}

gint64 peer_get_rate_to_peer(JsonObject * p) {
    return json_node_get_int(json_object_get_member(p, TPEER_RATE_TO_PEER));
}
