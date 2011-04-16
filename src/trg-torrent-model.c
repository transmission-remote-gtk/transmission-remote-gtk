/*
 * transmission-remote-gtk - Transmission RPC client for GTK
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

#include "config.h"
#include "torrent.h"
#include "tpeer.h"
#include "json.h"
#include "trg-torrent-model.h"
#include "protocol-constants.h"
#include "trg-model.h"
#include "util.h"

enum {
    TMODEL_TORRENT_COMPLETED,
    TMODEL_TORRENT_ADDED,
    TMODEL_TORRENT_ADDREMOVE,
    TMODEL_SIGNAL_COUNT
};

static guint signals[TMODEL_SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(TrgTorrentModel, trg_torrent_model, GTK_TYPE_LIST_STORE)
#define TRG_TORRENT_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_MODEL, TrgTorrentModelPrivate))
typedef struct _TrgTorrentModelPrivate TrgTorrentModelPrivate;

struct _TrgTorrentModelPrivate {
    GHashTable *ht;
};

static void trg_torrent_model_dispose(GObject * object)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(object);
    g_hash_table_destroy(priv->ht);
    G_OBJECT_CLASS(trg_torrent_model_parent_class)->dispose(object);
}

static guint32 torrent_get_flags(JsonObject * t, gint64 status);

static void
update_torrent_iter(TrgTorrentModel * model, gint64 serial,
                    GtkTreeIter * iter, JsonObject * t,
                    trg_torrent_model_update_stats * stats);

static void trg_torrent_model_class_init(TrgTorrentModelClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgTorrentModelPrivate));
    object_class->dispose = trg_torrent_model_dispose;

    signals[TMODEL_TORRENT_COMPLETED] =
        g_signal_new("torrent-completed",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgTorrentModelClass,
                                     torrent_completed), NULL,
                     NULL, g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    signals[TMODEL_TORRENT_ADDED] =
        g_signal_new("torrent-added",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgTorrentModelClass,
                                     torrent_added), NULL,
                     NULL, g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    signals[TMODEL_TORRENT_ADDREMOVE] =
        g_signal_new("torrent-addremove",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgTorrentModelClass,
                                     torrent_removed), NULL,
                     NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}

static void trg_torrent_model_count_peers(TrgTorrentModel * model,
                                          GtkTreeIter * iter,
                                          JsonObject * t)
{
    GList *peersList = json_array_get_elements(torrent_get_peers(t));
    gint seeders, leechers;
    GList *li;

    seeders = 0;
    leechers = 0;

    for (li = peersList; li; li = g_list_next(li)) {
        JsonObject *peer = json_node_get_object((JsonNode *) li->data);

        if (peer_get_is_downloading_from(peer))
            seeders++;

        if (peer_get_is_uploading_to(peer))
            leechers++;
    }

    g_list_free(peersList);

    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_SEEDS, seeders,
                       TORRENT_COLUMN_LEECHERS, leechers, -1);
}

static void trg_torrent_model_ref_free(gpointer data)
{
    GtkTreeRowReference *rr = (GtkTreeRowReference *) data;
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    if (path) {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter(model, &iter, path))
            gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

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
    column_types[TORRENT_COLUMN_SIZE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DONE] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_STATUS] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_SEEDS] = G_TYPE_INT;
    column_types[TORRENT_COLUMN_LEECHERS] = G_TYPE_INT;
    column_types[TORRENT_COLUMN_DOWNSPEED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_ADDED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_UPSPEED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_ETA] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_UPLOADED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DOWNLOADED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_RATIO] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_ID] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_JSON] = G_TYPE_POINTER;
    column_types[TORRENT_COLUMN_UPDATESERIAL] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FLAGS] = G_TYPE_INT;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self),
                                    TORRENT_COLUMN_COLUMNS, column_types);

    priv->ht =
        g_hash_table_new_full(g_int64_hash, g_int64_equal,
                              (GDestroyNotify) g_free,
                              trg_torrent_model_ref_free);
}

static guint32 torrent_get_flags(JsonObject * t, gint64 status)
{
    guint32 flags = 0;
    switch (status) {
    case STATUS_DOWNLOADING:
        flags |= TORRENT_FLAG_DOWNLOADING;
        break;
    case STATUS_PAUSED:
        flags |= TORRENT_FLAG_PAUSED;
        break;
    case STATUS_SEEDING:
        flags |= TORRENT_FLAG_SEEDING;
        break;
    case STATUS_CHECKING:
        flags |= TORRENT_FLAG_CHECKING;
        break;
    case STATUS_WAITING_TO_CHECK:
        flags |= TORRENT_FLAG_WAITING_CHECK;
        flags |= TORRENT_FLAG_CHECKING;
        break;
    }

    if (torrent_get_is_finished(t) == TRUE)
        flags |= TORRENT_FLAG_COMPLETE;
    else
        flags |= TORRENT_FLAG_INCOMPLETE;

    if (strlen(torrent_get_errorstr(t)) > 0)
        flags |= TORRENT_FLAG_ERROR;

    return flags;
}

static gboolean
trg_torrent_model_stats_scan_foreachfunc(GtkTreeModel * model,
                                         GtkTreePath * path G_GNUC_UNUSED,
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

    stats->count++;

    return FALSE;
}

static void
update_torrent_iter(TrgTorrentModel * model, gint64 serial,
                    GtkTreeIter * iter, JsonObject * t,
                    trg_torrent_model_update_stats * stats)
{
    guint lastFlags, newFlags;
    JsonObject *lastJson;
    gchar *statusString, *statusIcon;
    gint64 downRate, upRate, downloaded, uploaded, id, status;

    downRate = torrent_get_rate_down(t);
    stats->downRateTotal += downRate;

    upRate = torrent_get_rate_up(t);
    stats->upRateTotal += upRate;

    uploaded = torrent_get_uploaded(t);
    downloaded = torrent_get_downloaded(t);

    id = torrent_get_id(t);

    status = torrent_get_status(t);
    statusString = torrent_get_status_string(status);
    newFlags = torrent_get_flags(t, status);
    statusIcon = torrent_get_status_icon(newFlags);

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter,
                       TORRENT_COLUMN_FLAGS, &lastFlags,
                       TORRENT_COLUMN_JSON, &lastJson, -1);

    json_object_ref(t);

#ifdef DEBUG
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_ICON, statusIcon, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_NAME, torrent_get_name(t), -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_SIZE, torrent_get_size(t), -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_DONE, torrent_get_percent_done(t),
                       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_STATUS,
                       statusString, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_DOWNSPEED, downRate, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_FLAGS,
                       newFlags, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_UPSPEED,
                       upRate, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_ETA,
                       torrent_get_eta(t), -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_UPLOADED, uploaded, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_DOWNLOADED, downloaded, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_RATIO,
                       uploaded > 0
                       && downloaded >
                       0 ? (double) uploaded / (double) downloaded : 0,
                       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_ID, id,
                       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter, TORRENT_COLUMN_JSON, t,
                       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_UPDATESERIAL, serial, -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_ADDED, torrent_get_added_date(t),
                       -1);
#else
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       TORRENT_COLUMN_ICON, statusIcon,
                       TORRENT_COLUMN_ADDED, torrent_get_added_date(t),
                       TORRENT_COLUMN_NAME, torrent_get_name(t),
                       TORRENT_COLUMN_SIZE, torrent_get_size(t),
                       TORRENT_COLUMN_DONE,
                       torrent_get_percent_done(t),
                       TORRENT_COLUMN_STATUS, statusString,
                       TORRENT_COLUMN_DOWNSPEED, downRate,
                       TORRENT_COLUMN_FLAGS, newFlags,
                       TORRENT_COLUMN_UPSPEED, upRate,
                       TORRENT_COLUMN_ETA, torrent_get_eta(t),
                       TORRENT_COLUMN_UPLOADED, uploaded,
                       TORRENT_COLUMN_DOWNLOADED, downloaded,
                       TORRENT_COLUMN_RATIO,
                       uploaded >
                       0
                       && downloaded >
                       0 ? (double) uploaded / (double) downloaded : 0,
                       TORRENT_COLUMN_ID, id, TORRENT_COLUMN_JSON, t,
                       TORRENT_COLUMN_UPDATESERIAL, serial, -1);
#endif

    if (lastJson)
        json_object_unref(lastJson);

    if ((lastFlags & TORRENT_FLAG_DOWNLOADING)
        && (newFlags & TORRENT_FLAG_COMPLETE))
        g_signal_emit(model, signals[TMODEL_TORRENT_COMPLETED], 0, iter);

    trg_torrent_model_count_peers(model, iter, t);

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
                                   GtkTreePath * path G_GNUC_UNUSED,
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

void trg_torrent_model_update(TrgTorrentModel * model, trg_client * tc,
                              JsonObject * response,
                              trg_torrent_model_update_stats * stats,
                              gint mode)
{
    TrgTorrentModelPrivate *priv = TRG_TORRENT_MODEL_GET_PRIVATE(model);

    GList *torrentList;
    JsonObject *args, *t;
    GList *li;
    gint64 id;
    gint64 *idCopy;
    JsonArray *removedTorrents;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeRowReference *rr;
    gpointer *result;
    gboolean addRemove = FALSE;

    args = get_arguments(response);
    torrentList = json_array_get_elements(get_torrents(args));

    for (li = torrentList; li; li = g_list_next(li)) {
        t = json_node_get_object((JsonNode *) li->data);
        id = torrent_get_id(t);

        result =
            mode ==
            TORRENT_GET_MODE_FIRST ? NULL : g_hash_table_lookup(priv->ht,
                                                                &id);

        if (!result) {
            gtk_list_store_append(GTK_LIST_STORE(model), &iter);

            update_torrent_iter(model, tc->updateSerial, &iter, t, stats);

            path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
            rr = gtk_tree_row_reference_new(GTK_TREE_MODEL(model), path);
            idCopy = g_new(gint64, 1);
            *idCopy = id;
            g_hash_table_insert(priv->ht, idCopy, rr);
            gtk_tree_path_free(path);
            addRemove = TRUE;
            if (mode != TORRENT_GET_MODE_FIRST)
                g_signal_emit(model, signals[TMODEL_TORRENT_ADDED], 0,
                              &iter);
        } else {
            path = gtk_tree_row_reference_get_path((GtkTreeRowReference *)
                                                   result);
            if (path) {
                if (gtk_tree_model_get_iter
                    (GTK_TREE_MODEL(model), &iter, path)) {
                    update_torrent_iter(model, tc->updateSerial, &iter, t,
                                        stats);
                }
                gtk_tree_path_free(path);
            }
        }
    }

    g_list_free(torrentList);

    if (mode == TORRENT_GET_MODE_UPDATE) {
        GList *hitlist =
            trg_torrent_model_find_removed(GTK_TREE_MODEL(model),
                                           tc->updateSerial);
        if (hitlist) {
            for (li = hitlist; li; li = g_list_next(li)) {
                g_hash_table_remove(priv->ht, li->data);
                g_free(li->data);
            }
            addRemove = TRUE;
            g_list_free(hitlist);
        }
    } else if (mode > TORRENT_GET_MODE_FIRST) {
        removedTorrents = get_torrents_removed(args);
        if (removedTorrents) {
            for (li = json_array_get_elements(removedTorrents); li != NULL;
                 li = g_list_next(li)) {
                id = json_node_get_int((JsonNode *) li->data);
                g_hash_table_remove(priv->ht, &id);
                addRemove = TRUE;
            }
        }
    }

    if (addRemove)
        g_signal_emit(model, signals[TMODEL_TORRENT_ADDREMOVE], 0);

    gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                           trg_torrent_model_stats_scan_foreachfunc,
                           stats);
}
