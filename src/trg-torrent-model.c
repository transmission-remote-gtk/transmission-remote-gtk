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

#include <string.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <glib/gi18n.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "torrent.h"
#include "json.h"
#include "trg-torrent-model.h"
#include "protocol-constants.h"
#include "trg-model.h"
#include "util.h"

/* An extension of TrgModel (which is an extension of GtkListStore) which
 * updates from a JSON torrent-get response. It handles a number of different
 * update modes.
 *   1) The first update.
 *   2) A full update.
 *   3) An active-only update.
 *   4) Individual torrent updates.
 *
 * Other stuff it does.
 *   1) Populates a stats struct with speeds/state counts as it works through the
 *      response.
 *   2) Emits signals if something is added or removed. This is used by the state
 *      selector so it doesn't have to refresh itself on every update.
 *   3) Added or completed signals, for libnotify notifications.
 *   4) Maintains the torrent hash table (by ID).
 *      (and provide a lookup function which outputs an iter and/or JSON object.)
 *   5) If the download directory is new/changed, create a short version if there
 *      is one (duplicate it not) for the state selector to filter/populate against.
 *   6) Shorten the tracker announce URL.
 */

enum {
    TMODEL_TORRENT_COMPLETED,
    TMODEL_UPDATE,
    TMODEL_TORRENT_ADDED,
    TMODEL_STATE_CHANGED,
    TMODEL_SIGNAL_COUNT
};

#define PROP_REMOVE_IN_PROGRESS "remove-in-progress"

static guint signals[TMODEL_SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(TrgTorrentModel, trg_torrent_model, GTK_TYPE_LIST_STORE)
#define TRG_TORRENT_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_MODEL, TrgTorrentModelPrivate))
typedef struct _TrgTorrentModelPrivate TrgTorrentModelPrivate;

struct _TrgTorrentModelPrivate {
    GHashTable *ht;
    GRegex *urlHostRegex;
    trg_torrent_model_update_stats stats;
};

static void trg_torrent_model_dispose(GObject * object)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(object);
    g_hash_table_destroy(priv->ht);
    G_OBJECT_CLASS(trg_torrent_model_parent_class)->dispose(object);
}

static void
update_torrent_iter(TrgTorrentModel * model, TrgClient * tc, gint64 rpcv,
                    gint64 serial, GtkTreeIter * iter, JsonObject * t,
                    trg_torrent_model_update_stats * stats,
                    guint * whatsChanged);

static void trg_torrent_model_class_init(TrgTorrentModelClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgTorrentModelPrivate));
    object_class->dispose = trg_torrent_model_dispose;

    signals[TMODEL_TORRENT_COMPLETED] = g_signal_new("torrent-completed",
                                                     G_TYPE_FROM_CLASS
                                                     (object_class),
                                                     G_SIGNAL_RUN_LAST |
                                                     G_SIGNAL_ACTION,
                                                     G_STRUCT_OFFSET
                                                     (TrgTorrentModelClass,
                                                      torrent_completed),
                                                     NULL, NULL,
                                                     g_cclosure_marshal_VOID__POINTER,
                                                     G_TYPE_NONE, 1,
                                                     G_TYPE_POINTER);

    signals[TMODEL_UPDATE] = g_signal_new("update",
                                          G_TYPE_FROM_CLASS
                                          (object_class),
                                          G_SIGNAL_RUN_LAST |
                                          G_SIGNAL_ACTION,
                                          G_STRUCT_OFFSET
                                          (TrgTorrentModelClass,
                                           update),
                                          NULL, NULL,
                                          g_cclosure_marshal_VOID__VOID,
                                          G_TYPE_NONE, 0);

    signals[TMODEL_TORRENT_ADDED] = g_signal_new("torrent-added",
                                                 G_TYPE_FROM_CLASS
                                                 (object_class),
                                                 G_SIGNAL_RUN_LAST |
                                                 G_SIGNAL_ACTION,
                                                 G_STRUCT_OFFSET
                                                 (TrgTorrentModelClass,
                                                  torrent_added), NULL,
                                                 NULL,
                                                 g_cclosure_marshal_VOID__POINTER,
                                                 G_TYPE_NONE, 1,
                                                 G_TYPE_POINTER);

    signals[TMODEL_STATE_CHANGED] = g_signal_new("torrents-state-change",
                                                 G_TYPE_FROM_CLASS
                                                 (object_class),
                                                 G_SIGNAL_RUN_LAST |
                                                 G_SIGNAL_ACTION,
                                                 G_STRUCT_OFFSET
                                                 (TrgTorrentModelClass,
                                                  torrent_removed), NULL,
                                                 NULL,
                                                 g_cclosure_marshal_VOID__UINT,
                                                 G_TYPE_NONE, 1,
                                                 G_TYPE_UINT);
}

trg_torrent_model_update_stats *trg_torrent_model_get_stats(TrgTorrentModel
                                                            * model)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(model);
    return &(priv->stats);
}

static void
trg_torrent_model_count_peers(TrgTorrentModel * model,
                              GtkTreeIter * iter, JsonObject * t)
{
    GList *trackersList =
        json_array_get_elements(torrent_get_tracker_stats(t));
    gint64 seeders = 0;
    gint64 leechers = 0;
    gint64 downloads = 0;
    GList *li;

    for (li = trackersList; li; li = g_list_next(li)) {
        JsonObject *tracker = json_node_get_object((JsonNode *) li->data);

        seeders += tracker_stats_get_seeder_count(tracker);
        leechers += tracker_stats_get_leecher_count(tracker);
        downloads += tracker_stats_get_download_count(tracker);
    }

    g_list_free(trackersList);

    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_SEEDS,
                       seeders, TORRENT_COLUMN_LEECHERS, leechers,
                       TORRENT_COLUMN_DOWNLOADS, downloads, -1);
}

static void trg_torrent_model_ref_free(gpointer data)
{
    GtkTreeRowReference *rr = (GtkTreeRowReference *) data;
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    if (path) {
        GtkTreeIter iter;
        JsonObject *json;
        if (gtk_tree_model_get_iter(model, &iter, path)) {
            gtk_tree_model_get(model, &iter, TORRENT_COLUMN_JSON, &json,
                               -1);
            json_object_unref(json);
            g_object_set_data(G_OBJECT(model), PROP_REMOVE_IN_PROGRESS,
                              GINT_TO_POINTER(TRUE));
            gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
            g_object_set_data(G_OBJECT(model), PROP_REMOVE_IN_PROGRESS,
                              GINT_TO_POINTER(FALSE));
        }

        gtk_tree_path_free(path);
    }

    gtk_tree_row_reference_free(rr);
}

static void trg_torrent_model_init(TrgTorrentModel * self)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(self);

    GType column_types[TORRENT_COLUMN_COLUMNS];

    column_types[TORRENT_COLUMN_ICON] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_NAME] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_ERROR] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_SIZEWHENDONE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_TOTALSIZE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_HAVE_UNCHECKED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_PERCENTDONE] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_METADATAPERCENTCOMPLETE] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_STATUS] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_SEEDS] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_LEECHERS] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DOWNLOADS] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DOWNSPEED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_ADDED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_UPSPEED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_ETA] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_UPLOADED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DOWNLOADED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_HAVE_VALID] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_RATIO] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_ID] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_JSON] = G_TYPE_POINTER;
    column_types[TORRENT_COLUMN_UPDATESERIAL] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FLAGS] = G_TYPE_INT;
    column_types[TORRENT_COLUMN_DOWNLOADDIR] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_DOWNLOADDIR_SHORT] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_BANDWIDTH_PRIORITY] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DONE_DATE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FROMPEX] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FROMDHT] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FROMTRACKERS] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FROMLTEP] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FROMRESUME] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FROMINCOMING] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_PEER_SOURCES] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_SEED_RATIO_LIMIT] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_SEED_RATIO_MODE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_PEERS_CONNECTED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_PEERS_FROM_US] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_WEB_SEEDS_TO_US] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_PEERS_TO_US] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_TRACKERHOST] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_QUEUE_POSITION] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_LASTACTIVE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FILECOUNT] = G_TYPE_UINT;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self),
                                    TORRENT_COLUMN_COLUMNS, column_types);

    priv->ht = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                     (GDestroyNotify) g_free,
                                     trg_torrent_model_ref_free);

    g_object_set_data(G_OBJECT(self), PROP_REMOVE_IN_PROGRESS,
                      GINT_TO_POINTER(FALSE));

    priv->urlHostRegex = trg_uri_host_regex_new();
}

gboolean trg_torrent_model_is_remove_in_progress(TrgTorrentModel * model)
{
    return (gboolean) GPOINTER_TO_INT(g_object_get_data
                                      (G_OBJECT(model),
                                       PROP_REMOVE_IN_PROGRESS));
}

static gboolean
trg_torrent_model_reload_dir_aliases_foreachfunc(GtkTreeModel * model,
                                                 GtkTreePath *
                                                 path G_GNUC_UNUSED,
                                                 GtkTreeIter * iter,
                                                 gpointer gdata)
{
    gchar *downloadDir, *shortDownloadDir;

    gtk_tree_model_get(model, iter, TORRENT_COLUMN_DOWNLOADDIR,
                       &downloadDir, -1);

    shortDownloadDir =
        shorten_download_dir((TrgClient *) gdata, downloadDir);

    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_DOWNLOADDIR_SHORT, shortDownloadDir,
                       -1);

    g_free(downloadDir);
    g_free(shortDownloadDir);

    return FALSE;
}

void
trg_torrent_model_reload_dir_aliases(TrgClient * tc, GtkTreeModel * model)
{
    gtk_tree_model_foreach(model,
                           trg_torrent_model_reload_dir_aliases_foreachfunc,
                           tc);
    g_signal_emit(model, signals[TMODEL_STATE_CHANGED], 0,
                  TORRENT_UPDATE_PATH_CHANGE);
}

static gboolean
trg_torrent_model_stats_scan_foreachfunc(GtkTreeModel *
                                         model,
                                         GtkTreePath *
                                         path
                                         G_GNUC_UNUSED,
                                         GtkTreeIter * iter,
                                         gpointer gdata)
{
    trg_torrent_model_update_stats *stats =
        (trg_torrent_model_update_stats *) gdata;
    guint flags;

    gtk_tree_model_get(model, iter, TORRENT_COLUMN_FLAGS, &flags, -1);

    if (flags & TORRENT_FLAG_SEEDING)
        stats->seeding++;
    else if (flags & TORRENT_FLAG_DOWNLOADING)
        stats->down++;
    else if (flags & TORRENT_FLAG_PAUSED)
        stats->paused++;

    if (flags & TORRENT_FLAG_ERROR)
        stats->error++;

    if (flags & TORRENT_FLAG_COMPLETE)
        stats->complete++;
    else
        stats->incomplete++;

    if (flags & TORRENT_FLAG_CHECKING)
        stats->checking++;

    if (flags & TORRENT_FLAG_ACTIVE)
        stats->active++;

    if (flags & TORRENT_FLAG_SEEDING_WAIT)
        stats->seed_wait++;

    if (flags & TORRENT_FLAG_DOWNLOADING_WAIT)
        stats->down_wait++;

    stats->count++;

    return FALSE;
}

void trg_torrent_model_remove_all(TrgTorrentModel * model)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(model);
    g_hash_table_remove_all(priv->ht);
    gtk_list_store_clear(GTK_LIST_STORE(model));
}

gchar *shorten_download_dir(TrgClient * tc, const gchar * downloadDir)
{
    TrgPrefs *prefs = trg_client_get_prefs(tc);
    JsonArray *labels =
        trg_prefs_get_array(prefs, TRG_PREFS_KEY_DESTINATIONS,
                            TRG_PREFS_CONNECTION);
    JsonObject *session = trg_client_get_session(tc);
    const gchar *defaultDownloadDir = session_get_download_dir(session);
    gchar *shortDownloadDir = NULL;

    if (labels) {
        GList *labelsList = json_array_get_elements(labels);
        if (labelsList) {
            GList *li;
            for (li = labelsList; li; li = g_list_next(li)) {
                JsonObject *labelObj = json_node_get_object((JsonNode *)
                                                            li->data);
                const gchar *labelDir =
                    json_object_get_string_member(labelObj,
                                                  TRG_PREFS_KEY_DESTINATIONS_SUBKEY_DIR);
                if (!g_strcmp0(downloadDir, labelDir)) {
                    const gchar *labelLabel =
                        json_object_get_string_member(labelObj,
                                                      TRG_PREFS_SUBKEY_LABEL);
                    shortDownloadDir = g_strdup(labelLabel);
                    break;
                }
            }
            g_list_free(labelsList);
        }
    }

    if (shortDownloadDir) {
        return shortDownloadDir;
    } else {
        if (!g_strcmp0(defaultDownloadDir, downloadDir))
            return g_strdup(_("Default"));

        if (g_str_has_prefix(downloadDir, defaultDownloadDir)) {
            int offset = strlen(defaultDownloadDir);
            if (*(downloadDir + offset) == '/')
                offset++;

            if (offset < strlen(downloadDir))
                return g_strdup(downloadDir + offset);
        }
    }

    return g_strdup(downloadDir);
}

static inline void
update_torrent_iter(TrgTorrentModel * model,
                    TrgClient * tc, gint64 rpcv,
                    gint64 serial, GtkTreeIter * iter,
                    JsonObject * t,
                    trg_torrent_model_update_stats *
                    stats, guint * whatsChanged)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(model);
    GtkListStore *ls = GTK_LIST_STORE(model);
    guint lastFlags, newFlags;
    JsonObject *lastJson, *pf;
    JsonArray *trackerStats;
    gchar *statusString, *statusIcon, *downloadDir;
    gint64 downRate, upRate, haveValid, uploaded, downloaded, id, status,
        lpd;
    guint fileCount;
    gchar *firstTrackerHost = NULL;
    gchar *peerSources = NULL;
    gchar *lastDownloadDir = NULL;

    downRate = torrent_get_rate_down(t);
    stats->downRateTotal += downRate;

    upRate = torrent_get_rate_up(t);
    stats->upRateTotal += upRate;

    uploaded = torrent_get_uploaded(t);
    downloaded = torrent_get_downloaded(t);
    haveValid = torrent_get_have_valid(t);

    downloadDir = (gchar *) torrent_get_download_dir(t);
    rm_trailing_slashes(downloadDir);

    id = torrent_get_id(t);
    status = torrent_get_status(t);
    fileCount = json_array_get_length(torrent_get_files(t));
    newFlags =
        torrent_get_flags(t, rpcv, status, fileCount, downRate, upRate);
    statusString = torrent_get_status_string(rpcv, status, newFlags);
    statusIcon = torrent_get_status_icon(rpcv, newFlags);
    pf = torrent_get_peersfrom(t);
    trackerStats = torrent_get_tracker_stats(t);

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter, TORRENT_COLUMN_FLAGS,
                       &lastFlags, TORRENT_COLUMN_JSON, &lastJson,
                       TORRENT_COLUMN_DOWNLOADDIR, &lastDownloadDir, -1);

    json_object_ref(t);

    if (json_array_get_length(trackerStats) > 0) {
        JsonObject *firstTracker =
            json_array_get_object_element(trackerStats,
                                          0);
        firstTrackerHost = trg_gregex_get_first(priv->urlHostRegex,
                                                tracker_stats_get_host
                                                (firstTracker));
    }

    lpd = peerfrom_get_lpd(pf);
    if (newFlags & TORRENT_FLAG_ACTIVE) {
        if (lpd >= 0) {
            peerSources =
                g_strdup_printf("%" G_GINT64_FORMAT " / %" G_GINT64_FORMAT
                                " / %" G_GINT64_FORMAT " / %"
                                G_GINT64_FORMAT " / %" G_GINT64_FORMAT
                                " / %" G_GINT64_FORMAT " / %"
                                G_GINT64_FORMAT, peerfrom_get_trackers(pf),
                                peerfrom_get_incoming(pf),
                                peerfrom_get_ltep(pf),
                                peerfrom_get_dht(pf), peerfrom_get_pex(pf),
                                lpd, peerfrom_get_resume(pf));
        } else {
            peerSources =
                g_strdup_printf("%" G_GINT64_FORMAT " / %" G_GINT64_FORMAT
                                " / %" G_GINT64_FORMAT " / %"
                                G_GINT64_FORMAT " / %" G_GINT64_FORMAT
                                " / N/A / %" G_GINT64_FORMAT,
                                peerfrom_get_trackers(pf),
                                peerfrom_get_incoming(pf),
                                peerfrom_get_ltep(pf),
                                peerfrom_get_dht(pf), peerfrom_get_pex(pf),
                                peerfrom_get_resume(pf));
        }
    }
#ifdef DEBUG
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_ICON, statusIcon, -1);
    gtk_list_store_set(ls, iter,
                       TORRENT_COLUMN_NAME, torrent_get_name(t), -1);
    gtk_list_store_set(ls, iter,
                       TORRENT_COLUMN_SIZEWHENDONE,
                       torrent_get_size_when_done(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_PERCENTDONE,
                       (newFlags & TORRENT_FLAG_CHECKING) ?
                       torrent_get_recheck_progress(t)
                       : torrent_get_percent_done(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_STATUS, statusString, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_DOWNSPEED, downRate, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_FLAGS, newFlags, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_UPSPEED, upRate, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_ETA, torrent_get_eta(t),
                       -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_UPLOADED, uploaded, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_DOWNLOADED, downloaded,
                       -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_RATIO, uploaded > 0
                       && downloaded >
                       0 ? (double) uploaded / (double) downloaded : 0,
                       -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_ID, id, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_JSON, t, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_UPDATESERIAL, serial, -1);
    gtk_list_store_set(ls, iter,
                       TORRENT_COLUMN_ADDED, torrent_get_added_date(t),
                       -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_DOWNLOADDIR, downloadDir,
                       -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_BANDWIDTH_PRIORITY,
                       torrent_get_bandwidth_priority(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_TOTALSIZE,
                       torrent_get_total_size(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_HAVE_UNCHECKED,
                       torrent_get_have_unchecked(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_METADATAPERCENTCOMPLETE,
                       torrent_get_metadata_percent_complete(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_PEERS_TO_US,
                       torrent_get_peers_sending_to_us(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_PEERS_FROM_US,
                       torrent_get_peers_getting_from_us(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_WEB_SEEDS_TO_US,
                       torrent_get_web_seeds_sending_to_us(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_SEED_RATIO_LIMIT,
                       torrent_get_seed_ratio_limit(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_SEED_RATIO_MODE,
                       torrent_get_seed_ratio_mode(t), -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_FILECOUNT, fileCount, -1);
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_HAVE_VALID, haveValid, -1);
#else
    gtk_list_store_set(ls, iter, TORRENT_COLUMN_ICON, statusIcon,
                       TORRENT_COLUMN_ADDED, torrent_get_added_date(t),
                       TORRENT_COLUMN_FILECOUNT,
                       fileCount,
                       TORRENT_COLUMN_DONE_DATE, torrent_get_done_date(t),
                       TORRENT_COLUMN_NAME, torrent_get_name(t),
                       TORRENT_COLUMN_ERROR, torrent_get_error(t),
                       TORRENT_COLUMN_SIZEWHENDONE,
                       torrent_get_size_when_done(t),
                       TORRENT_COLUMN_PERCENTDONE,
                       (newFlags & TORRENT_FLAG_CHECKING) ?
                       torrent_get_recheck_progress(t)
                       : torrent_get_percent_done(t),
                       TORRENT_COLUMN_METADATAPERCENTCOMPLETE,
                       torrent_get_metadata_percent_complete(t),
                       TORRENT_COLUMN_STATUS, statusString,
                       TORRENT_COLUMN_DOWNSPEED, downRate,
                       TORRENT_COLUMN_FLAGS, newFlags,
                       TORRENT_COLUMN_UPSPEED, upRate, TORRENT_COLUMN_ETA,
                       torrent_get_eta(t), TORRENT_COLUMN_UPLOADED,
                       uploaded, TORRENT_COLUMN_DOWNLOADED, downloaded,
                       TORRENT_COLUMN_TOTALSIZE, torrent_get_total_size(t),
                       TORRENT_COLUMN_HAVE_UNCHECKED,
                       torrent_get_have_unchecked(t),
                       TORRENT_COLUMN_HAVE_VALID, haveValid,
                       TORRENT_COLUMN_FROMPEX, peerfrom_get_pex(pf),
                       TORRENT_COLUMN_FROMDHT, peerfrom_get_dht(pf),
                       TORRENT_COLUMN_FROMTRACKERS,
                       peerfrom_get_trackers(pf), TORRENT_COLUMN_FROMLTEP,
                       peerfrom_get_ltep(pf), TORRENT_COLUMN_FROMRESUME,
                       peerfrom_get_resume(pf),
                       TORRENT_COLUMN_FROMINCOMING,
                       peerfrom_get_incoming(pf),
                       TORRENT_COLUMN_PEER_SOURCES, peerSources,
                       TORRENT_COLUMN_PEERS_CONNECTED,
                       torrent_get_peers_connected(t),
                       TORRENT_COLUMN_PEERS_TO_US,
                       torrent_get_peers_sending_to_us(t),
                       TORRENT_COLUMN_PEERS_FROM_US,
                       torrent_get_peers_getting_from_us(t),
                       TORRENT_COLUMN_WEB_SEEDS_TO_US,
                       torrent_get_web_seeds_sending_to_us(t),
                       TORRENT_COLUMN_QUEUE_POSITION,
                       torrent_get_queue_position(t),
                       TORRENT_COLUMN_SEED_RATIO_LIMIT,
                       torrent_get_seed_ratio_limit(t),
                       TORRENT_COLUMN_SEED_RATIO_MODE,
                       torrent_get_seed_ratio_mode(t),
                       TORRENT_COLUMN_LASTACTIVE,
                       torrent_get_activity_date(t), TORRENT_COLUMN_RATIO,
                       uploaded > 0
                       && haveValid >
                       0 ? (double) uploaded / (double) haveValid : 0,
                       TORRENT_COLUMN_DOWNLOADDIR, downloadDir,
                       TORRENT_COLUMN_BANDWIDTH_PRIORITY,
                       torrent_get_bandwidth_priority(t),
                       TORRENT_COLUMN_ID, id, TORRENT_COLUMN_JSON, t,
                       TORRENT_COLUMN_TRACKERHOST,
                       firstTrackerHost ? firstTrackerHost : "",
                       TORRENT_COLUMN_UPDATESERIAL, serial, -1);
#endif

    if (!lastDownloadDir || g_strcmp0(downloadDir, lastDownloadDir)) {
        gchar *shortDownloadDir = shorten_download_dir(tc, downloadDir);
        gtk_list_store_set(ls, iter, TORRENT_COLUMN_DOWNLOADDIR_SHORT,
                           shortDownloadDir, -1);
        g_free(shortDownloadDir);
        *whatsChanged |= TORRENT_UPDATE_PATH_CHANGE;
    }

    if (lastJson)
        json_object_unref(lastJson);

    if ((lastFlags & TORRENT_FLAG_DOWNLOADING)
        && (!(newFlags & TORRENT_FLAG_DOWNLOADING))
        && (newFlags & TORRENT_FLAG_COMPLETE))
        g_signal_emit(model, signals[TMODEL_TORRENT_COMPLETED], 0, iter);

    if (lastFlags != newFlags)
        *whatsChanged |= TORRENT_UPDATE_STATE_CHANGE;

    trg_torrent_model_count_peers(model, iter, t);

    if (firstTrackerHost)
        g_free(firstTrackerHost);

    if (peerSources)
        g_free(peerSources);

    g_free(lastDownloadDir);
    g_free(statusString);
    g_free(statusIcon);
}

TrgTorrentModel *trg_torrent_model_new(void)
{
    return g_object_new(TRG_TYPE_TORRENT_MODEL, NULL);
}

struct TrgModelRemoveData {
    GList *toRemove;
    gint64 currentSerial;
};

GHashTable *get_torrent_table(TrgTorrentModel * model)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(model);
    return priv->ht;
}

gboolean
trg_model_find_removed_foreachfunc(GtkTreeModel * model,
                                   GtkTreePath *
                                   path G_GNUC_UNUSED,
                                   GtkTreeIter * iter, gpointer gdata)
{
    struct TrgModelRemoveData *args = (struct TrgModelRemoveData *) gdata;
    gint64 rowSerial;

    gtk_tree_model_get(model, iter, TORRENT_COLUMN_UPDATESERIAL,
                       &rowSerial, -1);

    if (rowSerial != args->currentSerial) {
        gint64 *id = g_new(gint64, 1);
        gtk_tree_model_get(model, iter, TORRENT_COLUMN_ID, id, -1);
        args->toRemove = g_list_append(args->toRemove, id);
    }

    return FALSE;
}

GList *trg_torrent_model_find_removed(GtkTreeModel * model,
                                      gint64 currentSerial)
{
    struct TrgModelRemoveData args;
    args.toRemove = NULL;
    args.currentSerial = currentSerial;

    gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                           trg_model_find_removed_foreachfunc, &args);

    return args.toRemove;
}

gboolean
get_torrent_data(GHashTable * table, gint64 id, JsonObject ** t,
                 GtkTreeIter * out_iter)
{
    gpointer result = g_hash_table_lookup(table, &id);
    gboolean found = FALSE;

    if (result) {
        GtkTreeRowReference *rr = (GtkTreeRowReference *) result;
        GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
        GtkTreeIter iter;
        if (path) {
            GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
            gtk_tree_model_get_iter(model, &iter, path);
            if (out_iter)
                *out_iter = iter;
            if (t)
                gtk_tree_model_get(model, &iter, TORRENT_COLUMN_JSON, t,
                                   -1);
            found = TRUE;
            gtk_tree_path_free(path);
        }
    }

    return found;
}

static void
trg_torrent_model_stat_counts_clear(trg_torrent_model_update_stats * stats)
{
    stats->count = stats->down = stats->error = stats->paused =
        stats->seeding = stats->complete = stats->incomplete =
        stats->active = stats->checking = stats->seed_wait =
        stats->down_wait = 0;
}

trg_torrent_model_update_stats *trg_torrent_model_update(TrgTorrentModel *
                                                         model,
                                                         TrgClient * tc,
                                                         JsonObject *
                                                         response,
                                                         gint mode)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(model);

    GList *torrentList;
    JsonObject *args, *t;
    GList *li;
    gint64 id;
    gint64 serial = trg_client_get_serial(tc);
    JsonArray *removedTorrents;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeRowReference *rr;
    gpointer *result;
    guint whatsChanged = 0;

    gint64 rpcv = trg_client_get_rpc_version(tc);

    args = get_arguments(response);
    torrentList = json_array_get_elements(get_torrents(args));

    priv->stats.downRateTotal = 0;
    priv->stats.upRateTotal = 0;

    for (li = torrentList; li; li = g_list_next(li)) {
        t = json_node_get_object((JsonNode *) li->data);
        id = torrent_get_id(t);

        result =
            mode == TORRENT_GET_MODE_FIRST ? NULL :
            g_hash_table_lookup(priv->ht, &id);

        if (!result) {
            gint64 *idCopy;
            gtk_list_store_append(GTK_LIST_STORE(model), &iter);
            whatsChanged |= TORRENT_UPDATE_ADDREMOVE;

            update_torrent_iter(model, tc, rpcv, serial,
                                &iter, t, &(priv->stats), &whatsChanged);

            path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
            rr = gtk_tree_row_reference_new(GTK_TREE_MODEL(model), path);
            idCopy = g_new(gint64, 1);
            *idCopy = id;
            g_hash_table_insert(priv->ht, idCopy, rr);
            gtk_tree_path_free(path);

            if (mode != TORRENT_GET_MODE_FIRST)
                g_signal_emit(model, signals[TMODEL_TORRENT_ADDED], 0,
                              &iter);
        } else {
            path = gtk_tree_row_reference_get_path((GtkTreeRowReference *)
                                                   result);
            if (path) {
                if (gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter,
                                            path)) {
                    update_torrent_iter(model, tc, rpcv,
                                        serial, &iter,
                                        t, &(priv->stats), &whatsChanged);
                }
                gtk_tree_path_free(path);
            }
        }
    }

    g_list_free(torrentList);

    if (mode == TORRENT_GET_MODE_UPDATE) {
        GList *hitlist =
            trg_torrent_model_find_removed(GTK_TREE_MODEL(model), serial);
        if (hitlist) {
            for (li = hitlist; li; li = g_list_next(li)) {
                g_hash_table_remove(priv->ht, li->data);
                g_free(li->data);
            }
            whatsChanged |= TORRENT_UPDATE_ADDREMOVE;
            g_list_free(hitlist);
        }
    } else if (mode > TORRENT_GET_MODE_FIRST) {
        removedTorrents = get_torrents_removed(args);
        if (removedTorrents) {
            GList *hitlist = json_array_get_elements(removedTorrents);
            for (li = hitlist; li; li = g_list_next(li)) {
                id = json_node_get_int((JsonNode *) li->data);
                g_hash_table_remove(priv->ht, &id);
                whatsChanged |= TORRENT_UPDATE_ADDREMOVE;
            }
            g_list_free(hitlist);
        }
    }

    if (whatsChanged != 0) {
        if ((whatsChanged & TORRENT_UPDATE_ADDREMOVE)
            || (whatsChanged & TORRENT_UPDATE_STATE_CHANGE)) {
            trg_torrent_model_stat_counts_clear(&priv->stats);
            gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                                   trg_torrent_model_stats_scan_foreachfunc,
                                   &(priv->stats));
        }
        g_signal_emit(model, signals[TMODEL_STATE_CHANGED], 0,
                      whatsChanged);
    }

    g_signal_emit(model, signals[TMODEL_UPDATE], 0);

    return &(priv->stats);
}
