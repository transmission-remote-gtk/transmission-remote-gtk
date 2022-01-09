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

#include "config.h"

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "torrent.h"
#include "trg-client.h"
#include "trg-model.h"
#include "trg-trackers-model.h"

G_DEFINE_TYPE(TrgTrackersModel, trg_trackers_model, GTK_TYPE_LIST_STORE)
#define TRG_TRACKERS_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TRACKERS_MODEL, TrgTrackersModelPrivate))
typedef struct _TrgTrackersModelPrivate TrgTrackersModelPrivate;

struct _TrgTrackersModelPrivate {
    gint64 torrentId;
    gint64 accept;
};

void trg_trackers_model_set_no_selection(TrgTrackersModel * model)
{
    TrgTrackersModelPrivate *priv = TRG_TRACKERS_MODEL_GET_PRIVATE(model);
    priv->torrentId = -1;
}

gint64 trg_trackers_model_get_torrent_id(TrgTrackersModel * model)
{
    TrgTrackersModelPrivate *priv = TRG_TRACKERS_MODEL_GET_PRIVATE(model);
    return priv->torrentId;
}

void
trg_trackers_model_update(TrgTrackersModel * model,
                          gint64 updateSerial, JsonObject * t, gint mode)
{
    TrgTrackersModelPrivate *priv = TRG_TRACKERS_MODEL_GET_PRIVATE(model);

    GtkTreeIter trackIter;
    JsonObject *tracker;
    gint64 trackerId;
    GList *trackers, *li;
    const gchar *announce;
    const gchar *scrape;

    if (mode == TORRENT_GET_MODE_FIRST) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
        priv->torrentId = torrent_get_id(t);
        priv->accept = TRUE;
    } else if (!priv->accept) {
        return;
    }

    trackers = json_array_get_elements(torrent_get_tracker_stats(t));

    for (li = trackers; li; li = g_list_next(li)) {
        tracker = json_node_get_object((JsonNode *) li->data);
        trackerId = tracker_stats_get_id(tracker);
        announce = tracker_stats_get_announce(tracker);
        scrape = tracker_stats_get_scrape(tracker);

        if (mode == TORRENT_GET_MODE_FIRST
            || find_existing_model_item(GTK_TREE_MODEL(model),
                                        TRACKERCOL_ID, trackerId,
                                        &trackIter) == FALSE)
            gtk_list_store_append(GTK_LIST_STORE(model), &trackIter);

#ifdef DEBUG
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ICON, "network-workgroup", -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_TIER,
                           tracker_stats_get_tier(tracker), -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ANNOUNCE, announce, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_SCRAPE, scrape, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ID, trackerId, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_UPDATESERIAL, updateSerial, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_LAST_ANNOUNCE_RESULT,
                           tracker_stats_get_announce_result(tracker), -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_LAST_ANNOUNCE_TIME,
                           tracker_stats_get_last_announce_time(tracker),
                           -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_LAST_SCRAPE_TIME,
                           tracker_stats_get_last_scrape_time(tracker),
                           -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_HOST,
                           tracker_stats_get_host(tracker), -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_LAST_ANNOUNCE_PEER_COUNT,
                           tracker_stats_get_last_announce_peer_count
                           (tracker), -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_LEECHERCOUNT,
                           tracker_stats_get_leecher_count(tracker), -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_SEEDERCOUNT,
                           tracker_stats_get_seeder_count(tracker), -1);
#else
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ICON, "network-workgroup",
                           TRACKERCOL_ID, trackerId,
                           TRACKERCOL_UPDATESERIAL, updateSerial,
                           TRACKERCOL_TIER,
                           tracker_stats_get_tier(tracker),
                           TRACKERCOL_ANNOUNCE, announce,
                           TRACKERCOL_SCRAPE, scrape, TRACKERCOL_HOST,
                           tracker_stats_get_host(tracker),
                           TRACKERCOL_LAST_ANNOUNCE_RESULT,
                           tracker_stats_get_announce_result(tracker),
                           TRACKERCOL_LAST_ANNOUNCE_TIME,
                           tracker_stats_get_last_announce_time(tracker),
                           TRACKERCOL_LAST_SCRAPE_TIME,
                           tracker_stats_get_last_scrape_time(tracker),
                           TRACKERCOL_LAST_ANNOUNCE_PEER_COUNT,
                           tracker_stats_get_last_announce_peer_count
                           (tracker), TRACKERCOL_LEECHERCOUNT,
                           tracker_stats_get_leecher_count(tracker),
                           TRACKERCOL_SEEDERCOUNT,
                           tracker_stats_get_seeder_count(tracker), -1);
#endif
    }

    g_list_free(trackers);

    trg_model_remove_removed(GTK_LIST_STORE(model),
                             TRACKERCOL_UPDATESERIAL, updateSerial);
}

static void trg_trackers_model_class_init(TrgTrackersModelClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgTrackersModelPrivate));
}

void
trg_trackers_model_set_accept(TrgTrackersModel * model, gboolean accept)
{
    TrgTrackersModelPrivate *priv = TRG_TRACKERS_MODEL_GET_PRIVATE(model);
    priv->accept = accept;
}

static void trg_trackers_model_init(TrgTrackersModel * self)
{
    TrgTrackersModelPrivate *priv = TRG_TRACKERS_MODEL_GET_PRIVATE(self);

    GType column_types[TRACKERCOL_COLUMNS];

    column_types[TRACKERCOL_ICON] = G_TYPE_STRING;
    column_types[TRACKERCOL_TIER] = G_TYPE_INT64;
    column_types[TRACKERCOL_ANNOUNCE] = G_TYPE_STRING;
    column_types[TRACKERCOL_SCRAPE] = G_TYPE_STRING;
    column_types[TRACKERCOL_ID] = G_TYPE_INT64;
    column_types[TRACKERCOL_LAST_ANNOUNCE_PEER_COUNT] = G_TYPE_INT64;
    column_types[TRACKERCOL_LAST_ANNOUNCE_TIME] = G_TYPE_INT64;
    column_types[TRACKERCOL_LAST_SCRAPE_TIME] = G_TYPE_INT64;
    column_types[TRACKERCOL_SEEDERCOUNT] = G_TYPE_INT64;
    column_types[TRACKERCOL_LEECHERCOUNT] = G_TYPE_INT64;
    column_types[TRACKERCOL_HOST] = G_TYPE_STRING;
    column_types[TRACKERCOL_LAST_ANNOUNCE_RESULT] = G_TYPE_STRING;
    column_types[TRACKERCOL_UPDATESERIAL] = G_TYPE_INT64;

    priv->accept = TRUE;
    priv->torrentId = -1;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self),
                                    TRACKERCOL_COLUMNS, column_types);
}

TrgTrackersModel *trg_trackers_model_new(void)
{
    return g_object_new(TRG_TYPE_TRACKERS_MODEL, NULL);
}
