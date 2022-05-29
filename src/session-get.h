/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011-2013  Alan Fitton

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

#ifndef SESSION_GET_H_
#define SESSION_GET_H_

#include <glib-object.h>

#define SGET_DOWNLOAD_DIR_FREE_SPACE      "download-dir-free-space"
#define SGET_BLOCKLIST_ENABLED            "blocklist-enabled"
#define SGET_BLOCKLIST_URL                "blocklist-url"
#define SGET_BLOCKLIST_SIZE               "blocklist-size"
#define SGET_DHT_ENABLED                  "dht-enabled"
#define SGET_LPD_ENABLED                  "lpd-enabled"
#define SGET_DOWNLOAD_DIR                 "download-dir"
#define SGET_INCOMPLETE_DIR               "incomplete-dir"
#define SGET_INCOMPLETE_DIR_ENABLED       "incomplete-dir-enabled"
#define SGET_ENCRYPTION                   "encryption"
#define SGET_PEER_LIMIT_GLOBAL            "peer-limit-global"
#define SGET_PEER_LIMIT_PER_TORRENT       "peer-limit-per-torrent"
#define SGET_PEER_PORT                    "peer-port"
#define SGET_PEER_PORT_RANDOM_ON_START    "peer-port-random-on-start"
#define SGET_PEX_ENABLED                  "pex-enabled"
#define SGET_PORT_FORWARDING_ENABLED      "port-forwarding-enabled"
#define SGET_RPC_VERSION                  "rpc-version"
#define SGET_RPC_VERSION_MINIMUM          "rpc-version-minimum"
#define SGET_SEED_RATIO_LIMIT             "seedRatioLimit"
#define SGET_SEED_RATIO_LIMITED           "seedRatioLimited"
#define SGET_SPEED_LIMIT_DOWN             "speed-limit-down"
#define SGET_SPEED_LIMIT_DOWN_ENABLED     "speed-limit-down-enabled"
#define SGET_SPEED_LIMIT_UP               "speed-limit-up"
#define SGET_SPEED_LIMIT_UP_ENABLED       "speed-limit-up-enabled"
#define SGET_VERSION                      "version"
#define SGET_RPC_VERSION                  "rpc-version"
#define SGET_TRASH_ORIGINAL_TORRENT_FILES "trash-original-torrent-files"
#define SGET_START_ADDED_TORRENTS         "start-added-torrents"
#define SGET_RENAME_PARTIAL_FILES         "rename-partial-files"
#define SGET_CACHE_SIZE_MB                "cache-size-mb"
#define SGET_SCRIPT_TORRENT_DONE_FILENAME "script-torrent-done-filename"
#define SGET_SCRIPT_TORRENT_DONE_ENABLED  "script-torrent-done-enabled"
#define SGET_BLOCKLIST_URL                "blocklist-url"
#define SGET_BLOCKLIST_ENABLED            "blocklist-enabled"
#define SGET_BLOCKLIST_SIZE               "blocklist-size"
#define SGET_DOWNLOAD_QUEUE_ENABLED       "download-queue-enabled"
#define SGET_DOWNLOAD_QUEUE_SIZE          "download-queue-size"
#define SGET_SEED_QUEUE_ENABLED           "seed-queue-enabled"
#define SGET_SEED_QUEUE_SIZE              "seed-queue-size"
#define SGET_QUEUE_STALLED_ENABLED        "queue-stalled-enabled"
#define SGET_QUEUE_STALLED_MINUTES        "queue-stalled-minutes"

#define SGET_ALT_SPEED_DOWN         "alt-speed-down"
#define SGET_ALT_SPEED_ENABLED      "alt-speed-enabled"
#define SGET_ALT_SPEED_TIME_BEGIN   "alt-speed-time-begin"
#define SGET_ALT_SPEED_TIME_ENABLED "alt-speed-time-enabled"
#define SGET_ALT_SPEED_TIME_END     "alt-speed-time-end"
#define SGET_ALT_SPEED_TIME_DAY     "alt-speed-time-day"
#define SGET_ALT_SPEED_UP           "alt-speed-up"

const gchar *session_get_torrent_done_filename(JsonObject *s);
gboolean session_get_torrent_done_enabled(JsonObject *s);
gint64 session_get_cache_size_mb(JsonObject *s);
const gchar *session_get_version_string(JsonObject *s);
gdouble session_get_version(JsonObject *s);
gboolean session_get_pex_enabled(JsonObject *s);
gboolean session_get_lpd_enabled(JsonObject *s);
const gchar *session_get_download_dir(JsonObject *s);
gboolean session_get_peer_port_random(JsonObject *s);
gint64 session_get_peer_port(JsonObject *s);
gint64 session_get_peer_limit_global(JsonObject *s);
gint64 session_get_peer_limit_per_torrent(JsonObject *s);
gboolean session_get_port_forwarding_enabled(JsonObject *s);
const gchar *session_get_blocklist_url(JsonObject *s);
gboolean session_get_blocklist_enabled(JsonObject *s);
gint64 session_get_blocklist_size(JsonObject *s);
gboolean session_get_rename_partial_files(JsonObject *s);
const gchar *session_get_encryption(JsonObject *s);
const gchar *session_get_incomplete_dir(JsonObject *s);
gboolean session_get_incomplete_dir_enabled(JsonObject *s);
gboolean session_get_seed_ratio_limited(JsonObject *s);
gdouble session_get_seed_ratio_limit(JsonObject *s);
gboolean session_get_start_added_torrents(JsonObject *s);
gboolean session_get_trash_original_torrent_files(JsonObject *s);
gboolean session_get_speed_limit_up_enabled(JsonObject *s);
gboolean session_get_speed_limit_alt_enabled(JsonObject *s);
gint64 session_get_speed_limit_up(JsonObject *s);
gboolean session_get_speed_limit_down_enabled(JsonObject *s);
gint64 session_get_speed_limit_down(JsonObject *s);
gboolean session_get_download_queue_enabled(JsonObject *s);
gint64 session_get_download_queue_size(JsonObject *s);
gboolean session_get_seed_queue_enabled(JsonObject *s);
gint64 session_get_seed_queue_size(JsonObject *s);
gint64 session_get_rpc_version(JsonObject *s);
gint64 session_get_download_dir_free_space(JsonObject *s);
gboolean session_get_dht_enabled(JsonObject *s);
gboolean session_get_alt_speed_enabled(JsonObject *s);
gint64 session_get_alt_speed_limit_up(JsonObject *s);
gint64 session_get_alt_speed_limit_down(JsonObject *s);

#endif /* SESSION_GET_H_ */
