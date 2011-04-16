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

#include "torrent.h"
#include "protocol-constants.h"
#include "util.h"

JsonArray *torrent_get_peers(JsonObject * t)
{
    return json_object_get_array_member(t, FIELD_PEERS);
}

JsonArray *torrent_get_wanted(JsonObject * t)
{
    return json_object_get_array_member(t, FIELD_WANTED);
}

JsonArray *torrent_get_priorities(JsonObject * t)
{
    return json_object_get_array_member(t, FIELD_PRIORITIES);
}

JsonArray *torrent_get_trackers(JsonObject * t)
{
    return json_object_get_array_member(t, FIELD_TRACKERS);
}

gint64 torrent_get_id(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ID);
}

const gchar *torrent_get_download_dir(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_DOWNLOAD_DIR);
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
    return json_object_get_double_member(t, FIELD_SEED_RATIO_LIMIT);
}

gint64 torrent_get_download_limit(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_DOWNLOAD_LIMIT);
}

gboolean torrent_get_download_limited(JsonObject * t)
{
    return json_object_get_boolean_member(t, FIELD_DOWNLOAD_LIMITED);
}

gint64 torrent_get_size(JsonObject * t)
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

gint64 torrent_get_downloaded(JsonObject * t)
{
    return torrent_get_have_valid(t) + torrent_get_have_unchecked(t);
}

gint64 torrent_get_status(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_STATUS);
}

gboolean torrent_get_is_finished(JsonObject * t)
{
    return torrent_get_left_until_done(t) <= 0;
}

gdouble torrent_get_percent_done(JsonObject * t)
{
    JsonNode *percentDone = json_object_get_member(t, FIELD_PERCENTDONE);
    GValue a = { 0 };
    json_node_get_value(percentDone, &a);
    switch (G_VALUE_TYPE(&a)) {
    case G_TYPE_INT64:
        return (gdouble) g_value_get_int64(&a) * 100.0;
    case G_TYPE_DOUBLE:
        return g_value_get_double(&a) * 100.0;
    default:
        return 0.0;
    }
}

gchar *torrent_get_status_icon(guint flags)
{
    if (flags & TORRENT_FLAG_ERROR)
        return g_strdup(GTK_STOCK_DIALOG_WARNING);
    else if (flags & TORRENT_FLAG_DOWNLOADING)
        return g_strdup(GTK_STOCK_GO_DOWN);
    else if (flags & TORRENT_FLAG_PAUSED)
        return g_strdup(GTK_STOCK_MEDIA_PAUSE);
    else if (flags & TORRENT_FLAG_SEEDING)
        return g_strdup(GTK_STOCK_GO_UP);
    else if (flags & TORRENT_FLAG_CHECKING)
        return g_strdup(GTK_STOCK_REFRESH);
    else
        return g_strdup(GTK_STOCK_DIALOG_QUESTION);
}

const gchar *torrent_get_errorstr(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_ERRORSTR);
}

gchar *torrent_get_status_string(gint64 value)
{
    switch (value) {
    case STATUS_DOWNLOADING:
        return g_strdup(_("Downloading"));
    case STATUS_PAUSED:
        return g_strdup(_("Paused"));
    case STATUS_SEEDING:
        return g_strdup(_("Seeding"));
    case STATUS_CHECKING:
        return g_strdup(_("Checking"));
    case STATUS_WAITING_TO_CHECK:
        return g_strdup(_("Waiting To Check"));
    default:
        return g_strdup(_("Unknown"));
    }
}

gboolean torrent_has_tracker(JsonObject * t, GRegex * rx, gchar * search)
{
    JsonArray *trackers = torrent_get_trackers(t);
    int i;

    for (i = 0; i < json_array_get_length(trackers); i++) {
        JsonObject *tracker = json_array_get_object_element(trackers, i);
        const gchar *trackerAnnounce = tracker_get_announce(tracker);
        gchar *trackerAnnounceHost =
            trg_gregex_get_first(rx, trackerAnnounce);
        int cmpResult = g_strcmp0(trackerAnnounceHost, search);
        g_free(trackerAnnounceHost);
        if (cmpResult == 0)
            return TRUE;
    }

    return FALSE;
}

gint64 tracker_get_id(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_ID);
}

gint64 tracker_get_tier(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_TIER);
}

gint64 torrent_get_left_until_done(JsonObject * t)
{
    return json_object_get_int_member(t, FIELD_LEFTUNTILDONE);
}

const gchar *tracker_get_announce(JsonObject * t)
{
    return json_object_get_string_member(t, FIELD_ANNOUNCE);
}

const gchar *tracker_get_scrape(JsonObject * t)
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
    return json_object_get_array_member(response, FIELD_TORRENTS);
}

JsonArray *torrent_get_files(JsonObject * args)
{
    return json_object_get_array_member(args, FIELD_FILES);
}
