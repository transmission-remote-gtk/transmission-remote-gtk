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

#include <gtk/gtk.h>

#include "trg-tree-view.h"
#include "trg-torrent-model.h"
#include "trg-torrent-tree-view.h"

G_DEFINE_TYPE(TrgTorrentTreeView, trg_torrent_tree_view,
	      TRG_TYPE_TREE_VIEW)

static void
trg_torrent_tree_view_class_init(TrgTorrentTreeViewClass *
				 klass G_GNUC_UNUSED)
{
}

static void trg_torrent_tree_view_init(TrgTorrentTreeView * tv)
{
    trg_tree_view_add_pixbuf_text_column(TRG_TREE_VIEW(tv),
					 TORRENT_COLUMN_ICON,
					 TORRENT_COLUMN_NAME, "Name", -1);
    trg_tree_view_add_size_column(TRG_TREE_VIEW(tv), "Size",
				  TORRENT_COLUMN_SIZE, -1);
    trg_tree_view_add_prog_column(TRG_TREE_VIEW(tv), "Done",
				  TORRENT_COLUMN_DONE, 70);
    trg_tree_view_add_column(TRG_TREE_VIEW(tv), "Status",
			     TORRENT_COLUMN_STATUS);
    trg_tree_view_add_column(TRG_TREE_VIEW(tv), "Seeds",
			     TORRENT_COLUMN_SEEDS);
    trg_tree_view_add_column(TRG_TREE_VIEW(tv), "Leechers",
			     TORRENT_COLUMN_LEECHERS);
    trg_tree_view_add_speed_column(TRG_TREE_VIEW(tv), "Down Speed",
				   TORRENT_COLUMN_DOWNSPEED, -1);
    trg_tree_view_add_speed_column(TRG_TREE_VIEW(tv), "Up Speed",
				   TORRENT_COLUMN_UPSPEED, -1);
    trg_tree_view_add_eta_column(TRG_TREE_VIEW(tv), "ETA",
				 TORRENT_COLUMN_ETA, -1);
    trg_tree_view_add_size_column(TRG_TREE_VIEW(tv), "Uploaded",
				  TORRENT_COLUMN_UPLOADED, -1);
    trg_tree_view_add_size_column(TRG_TREE_VIEW(tv), "Downloaded",
				  TORRENT_COLUMN_DOWNLOADED, -1);
    trg_tree_view_add_ratio_column(TRG_TREE_VIEW(tv), "Ratio",
				   TORRENT_COLUMN_RATIO, -1);
}

gint get_first_selected(trg_client * client, TrgTorrentTreeView * view,
			GtkTreeIter * iter, JsonObject ** json)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GList *selectionList;
    GList *firstNode;
    gint64 id = -1;
    gint64 updateSerial = -1;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    selectionList = gtk_tree_selection_get_selected_rows(selection, NULL);

    if ((firstNode = g_list_first(selectionList)) != NULL) {
	if (gtk_tree_model_get_iter(model, iter, firstNode->data) == TRUE) {
	    gtk_tree_model_get(model, iter,
			       TORRENT_COLUMN_JSON, json,
			       TORRENT_COLUMN_ID, &id,
			       TORRENT_COLUMN_UPDATESERIAL, &updateSerial,
			       -1);

	    if (updateSerial < client->updateSerial)
		id = -1;
	}
    }

    g_list_free(selectionList);

    return id;
}

void
trg_torrent_model_get_json_id_array_foreach(GtkTreeModel * model,
					    GtkTreePath *
					    path G_GNUC_UNUSED,
					    GtkTreeIter * iter,
					    gpointer data)
{
    JsonArray *output = (JsonArray *) data;
    gint64 id;
    gtk_tree_model_get(model, iter, TORRENT_COLUMN_ID, &id, -1);
    json_array_add_int_element(output, id);
}

JsonArray *build_json_id_array(TrgTorrentTreeView * tv)
{
    GtkTreeSelection *selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(tv));

    JsonArray *ids = json_array_new();
    gtk_tree_selection_selected_foreach(selection,
					(GtkTreeSelectionForeachFunc)
					trg_torrent_model_get_json_id_array_foreach,
					ids);
    return ids;
}

TrgTorrentTreeView *trg_torrent_tree_view_new(GtkTreeModel * model)
{
    GObject *obj = g_object_new(TRG_TYPE_TORRENT_TREE_VIEW, NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), model);

    return TRG_TORRENT_TREE_VIEW(obj);
}
