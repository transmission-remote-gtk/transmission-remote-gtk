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

#include <stdio.h>

#include <json-glib/json-glib.h>

#include "protocol-constants.h"
#include "json.h"
#include "session-get.h"

/* Just some functions to get fields out of a session-get response. */

const gchar *session_get_version_string(JsonObject * s)
{
    return json_object_get_string_member(s, SGET_VERSION);
}

gdouble session_get_version(JsonObject * s)
{
    const gchar *versionString = session_get_version_string(s);
    gchar *spaceChar = g_strrstr(" ", versionString);
    return g_ascii_strtod(versionString, &spaceChar);
}

gint64 session_get_download_dir_free_space(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_DOWNLOAD_DIR_FREE_SPACE);
}

gint64 session_get_rpc_version(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_RPC_VERSION);
}

gboolean session_get_pex_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_PEX_ENABLED);
}

gboolean session_get_lpd_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_LPD_ENABLED);
}

const gchar *session_get_download_dir(JsonObject * s)
{
    return json_object_get_string_member(s, SGET_DOWNLOAD_DIR);
}

gboolean session_get_peer_port_random(JsonObject * s)
{
    return json_object_get_boolean_member(s,
                                          SGET_PEER_PORT_RANDOM_ON_START);
}

gint64 session_get_peer_port(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_PEER_PORT);
}

gboolean session_get_port_forwarding_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_PORT_FORWARDING_ENABLED);
}

const gchar *session_get_blocklist_url(JsonObject * s)
{
    if (json_object_has_member(s, SGET_BLOCKLIST_URL))
        return json_object_get_string_member(s, SGET_BLOCKLIST_URL);
    else
        return NULL;
}

gint64 session_get_blocklist_size(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_BLOCKLIST_SIZE);
}

gboolean session_get_blocklist_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_BLOCKLIST_ENABLED);
}

gboolean session_get_rename_partial_files(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_RENAME_PARTIAL_FILES);
}

const gchar *session_get_encryption(JsonObject * s)
{
    return json_object_get_string_member(s, SGET_ENCRYPTION);
}

const gchar *session_get_incomplete_dir(JsonObject * s)
{
    return json_object_get_string_member(s, SGET_INCOMPLETE_DIR);
}

gboolean session_get_incomplete_dir_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_INCOMPLETE_DIR_ENABLED);
}

gboolean session_get_alt_speed_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_ALT_SPEED_ENABLED);
}

gboolean session_get_seed_ratio_limited(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_SEED_RATIO_LIMITED);
}

gboolean session_get_download_queue_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_DOWNLOAD_QUEUE_ENABLED);
}

gint64 session_get_download_queue_size(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_DOWNLOAD_QUEUE_SIZE);
}

gboolean session_get_seed_queue_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_SEED_QUEUE_ENABLED);
}

gint64 session_get_seed_queue_size(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_SEED_QUEUE_SIZE);
}

const gchar *session_get_torrent_done_filename(JsonObject * s)
{
    return json_object_get_string_member(s,
                                         SGET_SCRIPT_TORRENT_DONE_FILENAME);
}

gboolean session_get_torrent_done_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s,
                                          SGET_SCRIPT_TORRENT_DONE_ENABLED);
}

gint64 session_get_cache_size_mb(JsonObject * s)
{
    if (json_object_has_member(s, SGET_CACHE_SIZE_MB))
        return json_object_get_int_member(s, SGET_CACHE_SIZE_MB);
    else
        return -1;
}

gdouble session_get_seed_ratio_limit(JsonObject * s)
{
    return
        json_node_really_get_double(json_object_get_member
                                    (s, SGET_SEED_RATIO_LIMIT));
}

gboolean session_get_start_added_torrents(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_START_ADDED_TORRENTS);
}

gboolean session_get_trash_original_torrent_files(JsonObject * s)
{
    return json_object_get_boolean_member(s,
                                          SGET_TRASH_ORIGINAL_TORRENT_FILES);
}

gboolean session_get_speed_limit_alt_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_ALT_SPEED_ENABLED);
}

gboolean session_get_speed_limit_up_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_SPEED_LIMIT_UP_ENABLED);
}

gint64 session_get_peer_limit_per_torrent(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_PEER_LIMIT_PER_TORRENT);
}

gint64 session_get_peer_limit_global(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_PEER_LIMIT_GLOBAL);
}

gint64 session_get_alt_speed_limit_up(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_ALT_SPEED_UP);
}

gint64 session_get_speed_limit_up(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_SPEED_LIMIT_UP);
}

gboolean session_get_speed_limit_down_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s,
                                          SGET_SPEED_LIMIT_DOWN_ENABLED);
}

gint64 session_get_alt_speed_limit_down(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_ALT_SPEED_DOWN);
}

gint64 session_get_speed_limit_down(JsonObject * s)
{
    return json_object_get_int_member(s, SGET_SPEED_LIMIT_DOWN);
}

gboolean session_get_dht_enabled(JsonObject * s)
{
    return json_object_get_boolean_member(s, SGET_DHT_ENABLED);
}
