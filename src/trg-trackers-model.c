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

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "config.h"
#include "torrent.h"
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

void trg_trackers_model_update(TrgTrackersModel * model,
                               gint64 updateSerial, JsonObject * t,
                               gboolean first)
{
    TrgTrackersModelPrivate *priv = TRG_TRACKERS_MODEL_GET_PRIVATE(model);

    guint j;
    JsonArray *trackers;
    const gchar *announce;
    const gchar *scrape;

    if (first) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
        priv->torrentId = torrent_get_id(t);
        priv->accept = TRUE;
    } else if (!priv->accept) {
        return;
    }

    trackers = torrent_get_trackers(t);

    for (j = 0; j < json_array_get_length(trackers); j++) {
        GtkTreeIter trackIter;
        JsonObject *tracker =
            json_node_get_object(json_array_get_element(trackers, j));
        gint64 trackerId = tracker_get_id(tracker);

        announce = tracker_get_announce(tracker);
        scrape = tracker_get_scrape(tracker);

        if (first
            || find_existing_model_item(GTK_TREE_MODEL(model),
                                        TRACKERCOL_ID, trackerId,
                                        &trackIter) == FALSE)
            gtk_list_store_append(GTK_LIST_STORE(model), &trackIter);

#ifdef DEBUG
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ICON, GTK_STOCK_NETWORK, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_TIER, tracker_get_tier(tracker), -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ANNOUNCE, announce, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_SCRAPE, scrape, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ID, trackerId, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_UPDATESERIAL, updateSerial, -1);
#else
        gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
                           TRACKERCOL_ICON, GTK_STOCK_NETWORK,
                           TRACKERCOL_ID, trackerId,
                           TRACKERCOL_UPDATESERIAL, updateSerial,
                           TRACKERCOL_TIER,
                           tracker_get_tier(tracker),
                           TRACKERCOL_ANNOUNCE,
                           announce, TRACKERCOL_SCRAPE, scrape, -1);
#endif
    }

    trg_model_remove_removed(GTK_LIST_STORE(model),
                             TRACKERCOL_UPDATESERIAL, updateSerial);
}

static void trg_trackers_model_class_init(TrgTrackersModelClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgTrackersModelPrivate));
}

void trg_trackers_model_set_accept(TrgTrackersModel * model,
                                   gboolean accept)
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
