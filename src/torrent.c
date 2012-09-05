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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "json.h"
#include "torrent.h"
#include "protocol-constants.h"
#include "util.h"

/* Just some functions to get fields out of the torrent object. */

JsonArray *torrent_get_peers(JsonObject * t)
{
    g_assert(json_object_get_array_member(t, FIELD_PEERS));
    return json_object_get_array_member(t, FIELD_PEERS);
}

JsonObject *torrent_get_peersfrom(JsonObject * t)
{
    return json_object_get_object_member(t, FIELD_PEERSFROM);
}

JsonArray *torrent_get_wanted(JsonObject * t)
{
    g_assert(json_object_get_array_member(t, FIELD_WANTED));
    return json_object_get_array_member(t, FIELD_WANTED);
}

JsonArray *torrent_get_priorities(JsonObject * t)
{
    g_assert(json_object_get_array_member(t, FIELD_PRIORITIES));
    return json_object_get_array_member(t, FIELD_PRIORITIES);
}

JsonArray *torrent_get_tracker_stats(JsonObject * t)
{
    g_assert(json_object_get_array_member(t, FIELD_TRACKER_STATS));
    return json_object_get_array_member(t, FIELD_TRACKER_STATS);
}

gint64 torrent_get_id(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ID);
}

const gchar *torrent_get_download_dir(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_DOWNLOAD_DIR);
}

const gchar *torrent_get_comment(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_COMMENT);
}

gdouble torrent_get_metadata_percent_complete(JsonObject * t)
{
    JsonNode *node =
        json_object_get_member(t, FIELD_METADATAPERCENTCOMPLETE);
    if (node)
        return json_double_to_progress(node);
    else
        return 100.0;
}

const gchar *torrent_get_name(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_NAME);
}

gint64 torrent_get_added_date(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ADDED_DATE);
}

gboolean torrent_get_honors_session_limits(JsonObject * t)
{
    return json_object_get_boolean_member(t, FIELD_HONORS_SESSION_LIMITS);
}

gint64 torrent_get_bandwidth_priority(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_BANDWIDTH_PRIORITY);
}

gint64 torrent_get_upload_limit(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_UPLOAD_LIMIT);
}

gint64 torrent_get_peer_limit(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_PEER_LIMIT);
}

gboolean torrent_get_upload_limited(JsonObject * t)
{
    return json_object_get_boolean_member(t, FIELD_UPLOAD_LIMITED);
}

gint64 torrent_get_seed_ratio_mode(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_SEED_RATIO_MODE);
}

gdouble torrent_get_seed_ratio_limit(JsonObject * t)
{
    return
        json_node_really_get_double(json_object_get_member
                                    (t, FIELD_SEED_RATIO_LIMIT));
}

gint64 torrent_get_download_limit(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_DOWNLOAD_LIMIT);
}

gboolean torrent_get_download_limited(JsonObject * t)
{
    return json_object_get_boolean_member(t, FIELD_DOWNLOAD_LIMITED);
}

gint64 torrent_get_total_size(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_TOTAL_SIZE);
}

gint64 torrent_get_size_when_done(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_SIZEWHENDONE);
}

gint64 torrent_get_rate_down(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_RATEDOWNLOAD);
}

gint64 torrent_get_rate_up(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_RATEUPLOAD);
}

gint64 torrent_get_eta(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ETA);
}

gint64 torrent_get_downloaded(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_DOWNLOADEDEVER);
}

gint64 torrent_get_uploaded(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_UPLOADEDEVER);
}

gint64 torrent_get_have_valid(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_HAVEVALID);
}

gint64 torrent_get_have_unchecked(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_HAVEUNCHECKED);
}

gint64 torrent_get_status(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_STATUS);
}

gboolean torrent_get_is_finished(JsonObject * t)
{
    return torrent_get_left_until_done(t) <= 0;
}

gboolean torrent_get_is_private(JsonObject * t)
{
    return json_object_get_boolean_member(t, FIELD_ISPRIVATE);
}

gdouble torrent_get_percent_done(JsonObject * t)
{
    return
        json_double_to_progress(json_object_get_member
                                (t, FIELD_PERCENTDONE));
}

gdouble torrent_get_recheck_progress(JsonObject * t)
{
    return
        json_double_to_progress(json_object_get_member
                                (t, FIELD_RECHECK_PROGRESS));
}

gint64 torrent_get_activity_date(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ACTIVITY_DATE);
}

guint32
torrent_get_flags(JsonObject * t, gint64 rpcv, gint64 status,
                  gint64 fileCount, gint64 downRate, gint64 upRate)
{
    guint32 flags = 0;

    if (fileCount > 0 && torrent_get_is_finished(t) == TRUE)
        flags |= TORRENT_FLAG_COMPLETE;
    else
        flags |= TORRENT_FLAG_INCOMPLETE;

    if (rpcv >= NEW_STATUS_RPC_VERSION) {
        switch (status) {
        case TR_STATUS_STOPPED:
            flags |= TORRENT_FLAG_PAUSED;
            break;
        case TR_STATUS_CHECK_WAIT:
            flags |= TORRENT_FLAG_WAITING_CHECK;
            flags |= TORRENT_FLAG_CHECKING_ANY;
            break;
        case TR_STATUS_CHECK:
            flags |= TORRENT_FLAG_CHECKING;
            flags |= TORRENT_FLAG_CHECKING_ANY;
            break;
        case TR_STATUS_DOWNLOAD_WAIT:
            flags |= TORRENT_FLAG_DOWNLOADING_WAIT;
            flags |= TORRENT_FLAG_QUEUED;
            break;
        case TR_STATUS_DOWNLOAD:
            if (!(flags & TORRENT_FLAG_COMPLETE))
                flags |= TORRENT_FLAG_DOWNLOADING;

            if (fileCount <= 0)
                flags |= TORRENT_FLAG_DOWNLOADING_METADATA;

            flags |= TORRENT_FLAG_ACTIVE;
            break;
        case TR_STATUS_SEED_WAIT:
            flags |= TORRENT_FLAG_SEEDING_WAIT;
            break;
        case TR_STATUS_SEED:
            flags |= TORRENT_FLAG_SEEDING;
            if (torrent_get_peers_getting_from_us(t))
                flags |= TORRENT_FLAG_ACTIVE;
            break;
        }
    } else {
        switch (status) {
        case OLD_STATUS_DOWNLOADING:
            flags |= TORRENT_FLAG_DOWNLOADING;
            break;
        case OLD_STATUS_PAUSED:
            flags |= TORRENT_FLAG_PAUSED;
            break;
        case OLD_STATUS_SEEDING:
            flags |= TORRENT_FLAG_SEEDING;
            break;
        case OLD_STATUS_CHECKING:
            flags |= TORRENT_FLAG_CHECKING;
            break;
        case OLD_STATUS_WAITING_TO_CHECK:
            flags |= TORRENT_FLAG_WAITING_CHECK;
            flags |= TORRENT_FLAG_CHECKING;
            break;
        }

        if (downRate > 0 || upRate > 0)
            flags |= TORRENT_FLAG_ACTIVE;
    }

    if (torrent_get_error(t) > 0)
        flags |= TORRENT_FLAG_ERROR;

    return flags;
}

gchar *torrent_get_status_icon(gint64 rpcv, guint flags)
{
    if (flags & TORRENT_FLAG_ERROR)
        return g_strdup(GTK_STOCK_DIALOG_WARNING);
    else if (flags & TORRENT_FLAG_DOWNLOADING_METADATA)
        return g_strdup(GTK_STOCK_FIND);
    else if (flags & TORRENT_FLAG_DOWNLOADING)
        return g_strdup(GTK_STOCK_GO_DOWN);
    else if (flags & TORRENT_FLAG_PAUSED)
        return g_strdup(GTK_STOCK_MEDIA_PAUSE);
    else if (flags & TORRENT_FLAG_SEEDING)
        return g_strdup(GTK_STOCK_GO_UP);
    else if (flags & TORRENT_FLAG_CHECKING)
        return g_strdup(GTK_STOCK_REFRESH);
    else if (flags & TORRENT_FLAG_DOWNLOADING_WAIT)
        return g_strdup(GTK_STOCK_MEDIA_REWIND);
    else if (flags & TORRENT_FLAG_SEEDING_WAIT)
        return g_strdup(GTK_STOCK_MEDIA_FORWARD);
    else
        return g_strdup(GTK_STOCK_DIALOG_QUESTION);
}

gint64 torrent_get_done_date(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_DONE_DATE);
}

const gchar *torrent_get_errorstr(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_ERROR_STRING);
}

gint64 torrent_get_error(JsonObject * t)
{
    if (!json_object_has_member(t, FIELD_ERROR))
        return 0;
    else
        return json_object_get_int_member(t, FIELD_ERROR);
}

const gchar *torrent_get_creator(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_CREATOR);
}

gint64 torrent_get_date_created(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_DATE_CREATED);
}

const gchar *torrent_get_hash(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_HASH_STRING);
}

gchar *torrent_get_status_string(gint64 rpcv, gint64 value, guint flags)
{
    if (rpcv >= NEW_STATUS_RPC_VERSION) {
        switch (value) {
        case TR_STATUS_DOWNLOAD:
            if (flags & TORRENT_FLAG_DOWNLOADING_METADATA)
                return g_strdup(_("Metadata Downloading"));
            else
                return g_strdup(_("Downloading"));
        case TR_STATUS_DOWNLOAD_WAIT:
            return g_strdup(_("Queued download"));
        case TR_STATUS_CHECK_WAIT:
            return g_strdup(_("Waiting To Check"));
        case TR_STATUS_CHECK:
            return g_strdup(_("Checking"));
        case TR_STATUS_SEED_WAIT:
            return g_strdup(_("Queued seed"));
        case TR_STATUS_SEED:
            return g_strdup(_("Seeding"));
        case TR_STATUS_STOPPED:
            return g_strdup(_("Paused"));
        }
    } else {
        switch (value) {
        case OLD_STATUS_DOWNLOADING:
            if (flags & TORRENT_FLAG_DOWNLOADING_METADATA)
                return g_strdup(_("Metadata Downloading"));
            else
                return g_strdup(_("Downloading"));
        case OLD_STATUS_PAUSED:
            return g_strdup(_("Paused"));
        case OLD_STATUS_SEEDING:
            return g_strdup(_("Seeding"));
        case OLD_STATUS_CHECKING:
            return g_strdup(_("Checking"));
        case OLD_STATUS_WAITING_TO_CHECK:
            return g_strdup(_("Waiting To Check"));
        }
    }

    return g_strdup(_("Unknown"));
}

gboolean torrent_has_tracker(JsonObject * t, GRegex * rx, gchar * search)
{
    GList *trackers;
    GList *li;
    gboolean ret = FALSE;

    trackers = json_array_get_elements(torrent_get_tracker_stats(t));

    for (li = trackers; li; li = g_list_next(li)) {
        JsonObject *tracker = json_node_get_object((JsonNode *) li->data);
        const gchar *trackerAnnounce = tracker_stats_get_announce(tracker);
        gchar *trackerAnnounceHost =
            trg_gregex_get_first(rx, trackerAnnounce);
        int cmpResult = g_strcmp0(trackerAnnounceHost, search);
        g_free(trackerAnnounceHost);
        if (!cmpResult) {
            ret = TRUE;
            break;
        }
    }

    g_list_free(trackers);

    return ret;
}

gint64 torrent_get_left_until_done(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_LEFTUNTILDONE);
}

const gchar *tracker_stats_get_announce(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_ANNOUNCE);
}

const gchar *tracker_stats_get_scrape(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_SCRAPE);
}

JsonArray *get_torrents_removed(JsonObject * response)
{
    if (G_UNLIKELY(json_object_has_member(response, FIELD_REMOVED)))
        return json_object_get_array_member(response, FIELD_REMOVED);
    else
        return NULL;
}

JsonArray *get_torrents(JsonObject * response)
{
    g_assert(json_object_get_array_member(response, FIELD_TORRENTS));
    return json_object_get_array_member(response, FIELD_TORRENTS);
}

JsonArray *torrent_get_files(JsonObject * args)
{
    return json_object_get_array_member(args, FIELD_FILES);
}

gint64 torrent_get_peers_connected(JsonObject * args)
{
    return json_object_get_int_member(args, FIELD_PEERS_CONNECTED);
}

gint64 torrent_get_peers_sending_to_us(JsonObject * args)
{
    return json_object_get_int_member(args, FIELD_PEERS_SENDING_TO_US);
}

gint64 torrent_get_peers_getting_from_us(JsonObject * args)
{
    return json_object_get_int_member(args, FIELD_PEERS_GETTING_FROM_US);
}

gint64 torrent_get_web_seeds_sending_to_us(JsonObject * args)
{
    return json_object_get_int_member(args, FIELD_WEB_SEEDS_SENDING_TO_US);
}

gint64 torrent_get_queue_position(JsonObject * args)
{
    if (json_object_has_member(args, FIELD_QUEUE_POSITION))
        return json_object_get_int_member(args, FIELD_QUEUE_POSITION);
    else
        return -1;
}

/* tracker stats */

gint64 tracker_stats_get_id(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ID);
}

gint64 tracker_stats_get_tier(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_TIER);
}

gint64 tracker_stats_get_last_announce_peer_count(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_LAST_ANNOUNCE_PEER_COUNT);
}

gint64 tracker_stats_get_last_announce_time(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_LAST_ANNOUNCE_TIME);
}

gint64 tracker_stats_get_last_scrape_time(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_LAST_SCRAPE_TIME);
}

gint64 tracker_stats_get_seeder_count(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_SEEDERCOUNT);
}

gint64 tracker_stats_get_leecher_count(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_LEECHERCOUNT);
}

gint64 tracker_stats_get_download_count(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_DOWNLOADCOUNT);
}

const gchar *tracker_stats_get_announce_result(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_LAST_ANNOUNCE_RESULT);
}

const gchar *tracker_stats_get_host(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_HOST);
}

gchar *torrent_get_full_path(JsonObject * obj)
{
    const gchar *location =
        json_object_get_string_member(obj, FIELD_DOWNLOAD_DIR);
    const gchar *name = json_object_get_string_member(obj, FIELD_NAME);
    return g_strdup_printf("%s/%s", location, name);
}

gchar *torrent_get_full_dir(JsonObject * obj)
{
    gchar *containing_path, *name, *delim;
    const gchar *location;
    JsonArray *files = torrent_get_files(obj);
    JsonObject *firstFile;

    location = json_object_get_string_member(obj, FIELD_DOWNLOAD_DIR);
    firstFile = json_array_get_object_element(files, 0);
    name = g_strdup(json_object_get_string_member(firstFile, TFILE_NAME));

    if ((delim = g_strstr_len(name, -1, "/"))) {
        *delim = '\0';
        containing_path = g_strdup_printf("%s/%s", location, name);
    } else {
        containing_path = g_strdup(location);
    }

    g_free(name);
    return containing_path;
}

/* peers */

const gchar *peer_get_address(JsonObject * p)
{
    return json_object_get_string_member(p, TPEER_ADDRESS);
}

const gchar *peer_get_flagstr(JsonObject * p)
{
    return json_object_get_string_member(p, TPEER_FLAGSTR);
}

const gchar *peer_get_client_name(JsonObject * p)
{
    return json_object_get_string_member(p, TPEER_CLIENT_NAME);
}

gboolean peer_get_is_encrypted(JsonObject * p)
{
    return json_object_get_boolean_member(p, TPEER_IS_ENCRYPTED);
}

gboolean peer_get_is_uploading_to(JsonObject * p)
{
    return json_object_get_boolean_member(p, TPEER_IS_UPLOADING_TO);
}

gboolean peer_get_is_downloading_from(JsonObject * p)
{
    return json_object_get_boolean_member(p, TPEER_IS_DOWNLOADING_FROM);
}

gdouble peer_get_progress(JsonObject * p)
{
    return
        json_double_to_progress(json_object_get_member(p, TPEER_PROGRESS));
}

gint64 peer_get_rate_to_client(JsonObject * p)
{
    return json_object_get_int_member(p, TPEER_RATE_TO_CLIENT);
}

gint64 peer_get_rate_to_peer(JsonObject * p)
{
    return json_object_get_int_member(p, TPEER_RATE_TO_PEER);
}

gint64 peerfrom_get_pex(JsonObject * pf)
{
    return json_object_get_int_member(pf, TPEERFROM_FROMPEX);
}

gint64 peerfrom_get_dht(JsonObject * pf)
{
    return json_object_get_int_member(pf, TPEERFROM_FROMDHT);
}

gint64 peerfrom_get_trackers(JsonObject * pf)
{
    return json_object_get_int_member(pf, TPEERFROM_FROMTRACKERS);
}

gint64 peerfrom_get_ltep(JsonObject * pf)
{
    return json_object_get_int_member(pf, TPEERFROM_FROMLTEP);
}

gint64 peerfrom_get_resume(JsonObject * pf)
{
    return json_object_get_int_member(pf, TPEERFROM_FROMRESUME);
}

gint64 peerfrom_get_incoming(JsonObject * pf)
{
    return json_object_get_int_member(pf, TPEERFROM_FROMINCOMING);
}


gint64 peerfrom_get_lpd(JsonObject * pf)
{
    return json_object_has_member(pf, TPEERFROM_FROMLPD) ?
        json_object_get_int_member(pf, TPEERFROM_FROMLPD) : -1;
}

/* files */

gdouble file_get_progress(gint64 length, gint64 completed)
{
    if (length > 0) {
        gdouble progress =
            ((gdouble) completed / (gdouble) length) * 100.0;
        if (progress > 100.0)
            return 100.0;
        else
            return progress;
    } else {
        return 0.0;
    }
}

gint64 file_get_length(JsonObject * f)
{
    return json_object_get_int_member(f, TFILE_LENGTH);
}

gint64 file_get_bytes_completed(JsonObject * f)
{
    return json_object_get_int_member(f, TFILE_BYTES_COMPLETED);
}

const gchar *file_get_name(JsonObject * f)
{
    return json_object_get_string_member(f, TFILE_NAME);
}
