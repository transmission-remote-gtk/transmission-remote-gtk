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

#ifndef TORRENT_H_
#define TORRENT_H_

#include <json-glib/json-glib.h>

#include "trg-client.h"

#define TORRENT_FLAG_ERROR             (1 << 0)
#define TORRENT_FLAG_COMPLETE          (1 << 1)
#define TORRENT_FLAG_INCOMPLETE        (1 << 2)
#define TORRENT_FLAG_SEEDING           (1 << 3)
#define TORRENT_FLAG_SEEDING_WAIT      (1 << 4)
#define TORRENT_FLAG_CHECKING          (1 << 5)
#define TORRENT_FLAG_WAITING_CHECK     (1 << 6)
#define TORRENT_FLAG_CHECKING_ANY      (1 << 7)
#define TORRENT_FLAG_DOWNLOADING       (1 << 8)
#define TORRENT_FLAG_DOWNLOADING_WAIT  (1 << 9)
#define TORRENT_FLAG_PAUSED            (1 << 10)
#define TORRENT_FLAG_QUEUED            (1 << 11)
#define TORRENT_FLAG_ACTIVE            (1 << 12)
#define TORRENT_FLAG_DOWNLOADING_METADATA (1 << 13)
#define FILTER_FLAG_TRACKER            (1 << 14)
#define FILTER_FLAG_DIR                (1 << 15)

#define TORRENT_ADD_FLAG_PAUSED        (1 << 0) /* 0x01 */
#define TORRENT_ADD_FLAG_DELETE        (1 << 1) /* 0x02 */

gint64 torrent_get_total_size(JsonObject * t);
gint64 torrent_get_size_when_done(JsonObject * t);
const gchar *torrent_get_name(JsonObject * t);
gint64 torrent_get_rate_down(JsonObject * t);
gint64 torrent_get_rate_up(JsonObject * t);
gint64 torrent_get_eta(JsonObject * t);
gint64 torrent_get_uploaded(JsonObject * t);
gint64 torrent_get_downloaded(JsonObject * t);
const gchar *torrent_get_errorstr(JsonObject * t);
gint64 torrent_get_error(JsonObject * t);
const gchar *torrent_get_download_dir(JsonObject * t);
const gchar *torrent_get_comment(JsonObject * t);
gint64 torrent_get_have_unchecked(JsonObject * t);
gint64 torrent_get_have_valid(JsonObject * t);
gint64 torrent_get_status(JsonObject * t);
const gchar *torrent_get_creator(JsonObject * t);
gint64 torrent_get_date_created(JsonObject * t);
const gchar *torrent_get_hash(JsonObject * t);
gchar *torrent_get_status_string(gint64 rpcv, gint64 value, guint flags);
gchar *torrent_get_status_icon(gint64 rpcv, guint flags);
guint32 torrent_get_flags(JsonObject * t, gint64 rpcv, gint64 status,
                          gint64 fileCount, gint64 downRate,
                          gint64 upRate);
JsonArray *torrent_get_peers(JsonObject * t);
JsonObject *torrent_get_peersfrom(JsonObject * t);
JsonArray *torrent_get_tracker_stats(JsonObject * t);
JsonArray *torrent_get_wanted(JsonObject * t);
JsonArray *torrent_get_priorities(JsonObject * t);
gint64 torrent_get_id(JsonObject * t);
JsonArray *torrent_get_files(JsonObject * args);
gint64 torrent_get_peers_getting_from_us(JsonObject * args);
gint64 torrent_get_peers_sending_to_us(JsonObject * args);
gint64 torrent_get_web_seeds_sending_to_us(JsonObject * args);
gint64 torrent_get_peers_connected(JsonObject * args);
gdouble torrent_get_percent_done(JsonObject * t);
gdouble torrent_get_recheck_progress(JsonObject * t);
gint64 torrent_get_left_until_done(JsonObject * t);
gboolean torrent_get_is_finished(JsonObject * t);
gboolean torrent_get_is_private(JsonObject * t);
gboolean torrent_get_honors_session_limits(JsonObject * t);
gint64 torrent_get_bandwidth_priority(JsonObject * t);
gint64 torrent_get_upload_limit(JsonObject * t);
gint64 torrent_get_added_date(JsonObject * t);
gint64 torrent_get_done_date(JsonObject * t);
gboolean torrent_get_upload_limited(JsonObject * t);
gint64 torrent_get_download_limit(JsonObject * t);
gboolean torrent_get_download_limited(JsonObject * t);
gdouble torrent_get_seed_ratio_limit(JsonObject * t);
gint64 torrent_get_seed_ratio_mode(JsonObject * t);
gint64 torrent_get_peer_limit(JsonObject * t);
gboolean torrent_has_tracker(JsonObject * t, GRegex * rx, gchar * search);
gint64 torrent_get_queue_position(JsonObject * args);
gint64 torrent_get_activity_date(JsonObject * t);
gchar *torrent_get_full_dir(JsonObject * obj);
gchar *torrent_get_full_path(JsonObject * obj);
gdouble torrent_get_metadata_percent_complete(JsonObject * t);

/* outer response object */

JsonArray *get_torrents(JsonObject * response);
JsonArray *get_torrents_removed(JsonObject * response);

/* tracker stats */

const gchar *tracker_stats_get_announce(JsonObject * t);
const gchar *tracker_stats_get_scrape(JsonObject * t);
gint64 tracker_stats_get_tier(JsonObject * t);
gint64 tracker_stats_get_id(JsonObject * t);
gint64 tracker_stats_get_last_announce_peer_count(JsonObject * t);
gint64 tracker_stats_get_last_announce_time(JsonObject * t);
gint64 tracker_stats_get_seeder_count(JsonObject * t);
gint64 tracker_stats_get_leecher_count(JsonObject * t);
gint64 tracker_stats_get_download_count(JsonObject * t);
const gchar *tracker_stats_get_announce_result(JsonObject * t);
const gchar *tracker_stats_get_host(JsonObject * t);
gint64 tracker_stats_get_last_scrape_time(JsonObject * t);

/* files */

gint64 file_get_length(JsonObject * f);
gint64 file_get_bytes_completed(JsonObject * f);
const gchar *file_get_name(JsonObject * f);
gdouble file_get_progress(gint64 length, gint64 completed);

/* peers */

const gchar *peer_get_address(JsonObject * p);
const gchar *peer_get_client_name(JsonObject * p);
gboolean peer_get_is_encrypted(JsonObject * p);
gdouble peer_get_progress(JsonObject * p);
const gchar *peer_get_flagstr(JsonObject * p);
gint64 peer_get_rate_to_client(JsonObject * p);
gint64 peer_get_rate_to_peer(JsonObject * p);
gboolean peer_get_is_uploading_to(JsonObject * p);
gboolean peer_get_is_downloading_from(JsonObject * p);

gint64 peerfrom_get_pex(JsonObject * pf);
gint64 peerfrom_get_dht(JsonObject * pf);
gint64 peerfrom_get_trackers(JsonObject * pf);
gint64 peerfrom_get_ltep(JsonObject * pf);
gint64 peerfrom_get_resume(JsonObject * pf);
gint64 peerfrom_get_incoming(JsonObject * pf);
gint64 peerfrom_get_lpd(JsonObject * pf);
#endif                          /* TORRENT_H_ */
