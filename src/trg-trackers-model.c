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
#include <glib/gprintf.h>

#include "config.h"
#include "torrent.h"
#include "trg-trackers-model.h"

G_DEFINE_TYPE(TrgTrackersModel, trg_trackers_model, GTK_TYPE_LIST_STORE)

void trg_trackers_model_update(TrgTrackersModel * model, JsonObject * t)
{
    guint j;
    JsonArray *trackers;
    const gchar *announce;
    const gchar *scrape;

    trackers = torrent_get_trackers(t);

    gtk_list_store_clear(GTK_LIST_STORE(model));

    for (j = 0; j < json_array_get_length(trackers); j++) {
		GtkTreeIter trackIter;
		JsonObject *tracker =
			json_node_get_object(json_array_get_element(trackers, j));

		announce = tracker_get_announce(tracker);
		scrape = tracker_get_scrape(tracker);
#ifdef DEBUG
		g_printf("show tracker: announce=\"%s\"\n", announce);
		g_printf("show tracker: scrape=\"%s\"\n", scrape);
#endif

		if (announce == NULL || scrape == NULL)
			continue;

		gtk_list_store_append(GTK_LIST_STORE(model), &trackIter);
		gtk_list_store_set(GTK_LIST_STORE(model), &trackIter,
				   TRACKERCOL_ICON, GTK_STOCK_NETWORK,
				   TRACKERCOL_TIER,
				   tracker_get_tier(tracker),
				   TRACKERCOL_ANNOUNCE,
				   announce, TRACKERCOL_SCRAPE, scrape, -1);
    }
}

static void
trg_trackers_model_class_init(TrgTrackersModelClass * klass G_GNUC_UNUSED)
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
