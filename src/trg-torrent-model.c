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

#include "torrent.h"
#include "tpeer.h"
#include "json.h"
#include "trg-torrent-model.h"
#include "protocol-constants.h"
#include "trg-model.h"
#include "util.h"

enum {
    TMODEL_TORRENT_COMPLETED,
    TMODEL_SIGNAL_COUNT
};

static guint signals[TMODEL_SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(TrgTorrentModel, trg_torrent_model, GTK_TYPE_LIST_STORE)

static guint32 torrent_get_flags(JsonObject * t, gint64 status,
				 TrgTorrentModelClassUpdateStats * stats);

static void
update_torrent_iter(gint64 serial, TrgTorrentModel * model,
		    GtkTreeIter * iter, JsonObject * t,
		    TrgTorrentModelClassUpdateStats * stats);

static gboolean
find_existing_torrent_item_foreachfunc(GtkTreeModel * model,
				       GtkTreePath * path,
				       GtkTreeIter * iter, gpointer data);

static gboolean
find_existing_torrent_item(TrgTorrentModel * model, JsonObject * t,
			   GtkTreeIter * iter);

static void trg_torrent_model_class_init(TrgTorrentModelClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    signals[TMODEL_TORRENT_COMPLETED] =
	g_signal_new("torrent-completed",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		     G_STRUCT_OFFSET(TrgTorrentModelClass,
				     torrent_completed), NULL,
		     NULL, g_cclosure_marshal_VOID__POINTER,
		     G_TYPE_NONE, 1, G_TYPE_POINTER);

}

static void trg_torrent_module_count_peers(TrgTorrentModel * model,
					   GtkTreeIter * iter,
					   JsonObject * t)
{
    JsonArray *peers;
    gint seeders, leechers, j;

    peers = torrent_get_peers(t);

    seeders = 0;
    leechers = 0;

    for (j = 0; j < json_array_get_length(peers); j++) {
	JsonObject *peer;

	peer = json_node_get_object(json_array_get_element(peers, j));

	if (peer_get_is_downloading_from(peer))
	    seeders++;

	if (peer_get_is_uploading_to(peer))
	    leechers++;
    }

    gtk_list_store_set(GTK_LIST_STORE(model), iter,
		       TORRENT_COLUMN_SEEDS, seeders,
		       TORRENT_COLUMN_LEECHERS, leechers, -1);
}

static void trg_torrent_model_init(TrgTorrentModel * self)
{
    GType column_types[TORRENT_COLUMN_COLUMNS];

    column_types[TORRENT_COLUMN_ICON] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_NAME] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_SIZE] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DONE] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_STATUS] = G_TYPE_STRING;
    column_types[TORRENT_COLUMN_SEEDS] = G_TYPE_INT;
    column_types[TORRENT_COLUMN_LEECHERS] = G_TYPE_INT;
    column_types[TORRENT_COLUMN_DOWNSPEED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_UPSPEED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_ETA] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_UPLOADED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_DOWNLOADED] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_RATIO] = G_TYPE_DOUBLE;
    column_types[TORRENT_COLUMN_ID] = G_TYPE_INT;
    column_types[TORRENT_COLUMN_JSON] = G_TYPE_POINTER;
    column_types[TORRENT_COLUMN_UPDATESERIAL] = G_TYPE_INT64;
    column_types[TORRENT_COLUMN_FLAGS] = G_TYPE_INT;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self),
				    TORRENT_COLUMN_COLUMNS, column_types);
}

static guint32 torrent_get_flags(JsonObject * t, gint64 status,
				 TrgTorrentModelClassUpdateStats * stats)
{
    guint32 flags = 0;
    switch (status) {
    case STATUS_DOWNLOADING:
	flags |= TORRENT_FLAG_DOWNLOADING;
	stats->down++;
	break;
    case STATUS_PAUSED:
	flags |= TORRENT_FLAG_PAUSED;
	stats->paused++;
	break;
    case STATUS_SEEDING:
	flags |= TORRENT_FLAG_SEEDING;
	stats->seeding++;
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

static void
update_torrent_iter(gint64 serial, TrgTorrentModel * model,
		    GtkTreeIter * iter, JsonObject * t,
		    TrgTorrentModelClassUpdateStats * stats)
{
    guint lastFlags, newFlags;
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
    newFlags = torrent_get_flags(t, status, stats);
    statusIcon = torrent_get_status_icon(newFlags);

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter,
		       TORRENT_COLUMN_FLAGS, &lastFlags, -1);

    gtk_list_store_set(GTK_LIST_STORE(model), iter,
		       TORRENT_COLUMN_ICON, statusIcon,
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
		       0 ? (double) uploaded / (double) downloaded : 0,
		       TORRENT_COLUMN_ID, id, TORRENT_COLUMN_JSON, t,
		       TORRENT_COLUMN_UPDATESERIAL, serial, -1);


    if ((lastFlags & TORRENT_FLAG_DOWNLOADING) ==
	TORRENT_FLAG_DOWNLOADING
	&& (newFlags & TORRENT_FLAG_COMPLETE) == TORRENT_FLAG_COMPLETE)
	g_signal_emit(model, signals[TMODEL_TORRENT_COMPLETED], 0, iter);

    trg_torrent_module_count_peers(model, iter, t);

    g_free(statusString);
    g_free(statusIcon);
}

TrgTorrentModel *trg_torrent_model_new(void)
{
    return g_object_new(TRG_TYPE_TORRENT_MODEL, NULL);
}

static gboolean
find_existing_torrent_item_foreachfunc(GtkTreeModel * model,
				       GtkTreePath * path,
				       GtkTreeIter * iter, gpointer data)
{
    struct idAndIter *ii;
    gint currentId;

    ii = (struct idAndIter *) data;

    gtk_tree_model_get(model, iter, TORRENT_COLUMN_ID, &currentId, -1);
    if (currentId == ii->id) {
	ii->iter = iter;
	return ii->found = TRUE;
    }

    return FALSE;
}

static gboolean
find_existing_torrent_item(TrgTorrentModel * model, JsonObject * t,
			   GtkTreeIter * iter)
{
    struct idAndIter ii;
    ii.id = torrent_get_id(t);
    ii.found = FALSE;
    gtk_tree_model_foreach(GTK_TREE_MODEL(model),
			   find_existing_torrent_item_foreachfunc, &ii);
    if (ii.found == TRUE)
	*iter = *(ii.iter);
    return ii.found;
}

void trg_torrent_model_update(TrgTorrentModel * model, trg_client * tc,
			      JsonObject * response,
			      TrgTorrentModelClassUpdateStats * stats,
			      gboolean first)
{
    int i;
    JsonArray *newTorrents;

    newTorrents = get_torrents(get_arguments(response));

    stats->count = json_array_get_length(newTorrents);

    for (i = 0; i < stats->count; i++) {
	GtkTreeIter iter;
	JsonObject *t;

	t = json_array_get_object_element(newTorrents, i);

	if (first == TRUE
	    || find_existing_torrent_item(model, t, &iter) == FALSE)
	    gtk_list_store_append(GTK_LIST_STORE(model), &iter);

	update_torrent_iter(tc->updateSerial, model, &iter, t, stats);
    }

    json_array_ref(newTorrents);

    if (tc->torrents != NULL)
	json_array_unref(tc->torrents);

    tc->torrents = newTorrents;

    trg_model_remove_removed(GTK_LIST_STORE(model),
			     TORRENT_COLUMN_UPDATESERIAL,
			     tc->updateSerial);
}
