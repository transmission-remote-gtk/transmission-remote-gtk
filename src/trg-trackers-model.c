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

#include "torrent.h"
#include "trg-trackers-model.h"

G_DEFINE_TYPE(TrgTrackersModel, trg_trackers_model, GTK_TYPE_LIST_STORE)

void trg_trackers_model_update(TrgTrackersModel * model, JsonObject * t)
{
    int j;
    JsonArray *trackers;

    gtk_list_store_clear(GTK_LIST_STORE(model));

    trackers = torrent_get_trackers(t);
    for (j = 0; j < json_array_get_length(trackers); j++) {
	JsonObject *tracker =
	    json_node_get_object(json_array_get_element(trackers, j));
	GtkTreeIter trackIter;
	gtk_list_store_append(GTK_LIST_STORE(model), &trackIter);
	gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
			   TRACKERCOL_ICON, GTK_STOCK_NETWORK,
			   TRACKERCOL_TIER,
			   tracker_get_tier(tracker),
			   TRACKERCOL_ANNOUNCE,
			   tracker_get_announce(tracker),
			   TRACKERCOL_SCRAPE,
			   tracker_get_scrape(tracker), -1);
    }
}

static void trg_trackers_model_class_init(TrgTrackersModelClass * klass)
{
}

static void trg_trackers_model_init(TrgTrackersModel * self)
{
    GType column_types[TRACKERCOL_COLUMNS];

    column_types[TRACKERCOL_ICON] = G_TYPE_STRING;
    column_types[TRACKERCOL_TIER] = G_TYPE_INT;
    column_types[TRACKERCOL_ANNOUNCE] = G_TYPE_STRING;
    column_types[TRACKERCOL_SCRAPE] = G_TYPE_STRING;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self),
				    TRACKERCOL_COLUMNS, column_types);
}

TrgTrackersModel *trg_trackers_model_new(void)
{
    return g_object_new(TRG_TYPE_TRACKERS_MODEL, NULL);
}
