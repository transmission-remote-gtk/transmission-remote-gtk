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
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "trg-main-window.h"
#include "trg-torrent-move-dialog.h"
#include "hig.h"
#include "torrent.h"
#include "requests.h"
#include "dispatch.h"

G_DEFINE_TYPE(TrgTorrentMoveDialog, trg_torrent_move_dialog,
	      GTK_TYPE_DIALOG)
#define TRG_TORRENT_MOVE_DIALOG_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_MOVE_DIALOG, TrgTorrentMoveDialogPrivate))
typedef struct _TrgTorrentMoveDialogPrivate
 TrgTorrentMoveDialogPrivate;

struct _TrgTorrentMoveDialogPrivate {
    trg_client *client;
    TrgMainWindow *win;
    JsonArray *ids;
    GtkWidget *location_combo, *move_check, *move_button;
};

static void
trg_torrent_move_dialog_class_init(TrgTorrentMoveDialogClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgTorrentMoveDialogPrivate));
}

static void
trg_torrent_move_response_cb(GtkDialog * dlg, gint res_id, gpointer data)
{
    TrgTorrentMoveDialogPrivate *priv =
	TRG_TORRENT_MOVE_DIALOG_GET_PRIVATE(dlg);
    if (res_id == GTK_RESPONSE_ACCEPT) {
	gchar *location =
	    gtk_combo_box_get_active_text(GTK_COMBO_BOX
					  (priv->location_combo));
	JsonNode *request = torrent_set_location(priv->ids, location,
						 gtk_toggle_button_get_active
						 (GTK_TOGGLE_BUTTON
						  (priv->move_check)));
	g_free(location);
	dispatch_async(priv->client, request,
		       on_generic_interactive_action, data);
    } else {
	json_array_unref(priv->ids);
    }
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void location_changed(GtkWidget * w, gpointer data)
{
    TrgTorrentMoveDialogPrivate *priv =
	TRG_TORRENT_MOVE_DIALOG_GET_PRIVATE(data);
    gchar *location =
	gtk_combo_box_get_active_text(GTK_COMBO_BOX(priv->location_combo));
    gtk_widget_set_sensitive(priv->move_button, strlen(location) > 0);
    g_free(location);
}

static void trg_torrent_move_dialog_init(TrgTorrentMoveDialog * self)
{
    TrgTorrentMoveDialogPrivate *priv =
	TRG_TORRENT_MOVE_DIALOG_GET_PRIVATE(self);
    GtkWidget *w, *t;
    gint row = 0;

    t = hig_workarea_create();

    w = priv->location_combo = gtk_combo_box_entry_new_text();
    g_signal_connect(w, "changed", G_CALLBACK(location_changed), self);
    w = hig_workarea_add_row(t, &row, "Location:", w, NULL);

    priv->move_check =
	hig_workarea_add_wide_checkbutton(t, &row, "Move", TRUE);

    gtk_window_set_destroy_with_parent(GTK_WINDOW(self), TRUE);

    gtk_dialog_add_button(GTK_DIALOG(self), GTK_STOCK_CLOSE,
			  GTK_RESPONSE_CANCEL);
    priv->move_button =
	gtk_dialog_add_button(GTK_DIALOG(self), "Move",
			      GTK_RESPONSE_ACCEPT);
    gtk_widget_set_sensitive(priv->move_button, FALSE);

    gtk_container_set_border_width(GTK_CONTAINER(self), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(self), GTK_RESPONSE_ACCEPT);

    gtk_dialog_set_alternative_button_order(GTK_DIALOG(self),
					    GTK_RESPONSE_ACCEPT,
					    GTK_RESPONSE_CANCEL, -1);

    gtk_container_set_border_width(GTK_CONTAINER(t), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(self)->vbox), t, TRUE, TRUE, 0);
}

TrgTorrentMoveDialog *trg_torrent_move_dialog_new(TrgMainWindow * win,
						  trg_client * client,
						  TrgTorrentTreeView * ttv)
{
    GObject *obj = g_object_new(TRG_TYPE_TORRENT_MOVE_DIALOG, NULL);
    TrgTorrentMoveDialogPrivate *priv =
	TRG_TORRENT_MOVE_DIALOG_GET_PRIVATE(obj);

    gint count;
    gchar *msg;

    priv->client = client;
    priv->win = win;
    priv->ids = build_json_id_array(ttv);

    count =
	gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection
					       (GTK_TREE_VIEW(ttv)));

    if (count == 1) {
	GtkTreeIter iter;
	JsonObject *json;
	gchar *name;
	const gchar *current_location;
	get_first_selected(client, ttv, &iter, &json);
	gtk_tree_model_get(gtk_tree_view_get_model(GTK_TREE_VIEW(ttv)),
			   &iter, TORRENT_COLUMN_NAME, &name, -1);
	current_location = torrent_get_download_dir(json);
	gtk_combo_box_append_text(GTK_COMBO_BOX(priv->location_combo),
				  current_location);
	gtk_combo_box_set_active(GTK_COMBO_BOX(priv->location_combo), 0);
	msg = g_strdup_printf("Move %s", name);
	g_free(name);
    } else {
	msg = g_strdup_printf("Move %d torrents", count);
    }

    gtk_window_set_transient_for(GTK_WINDOW(obj), GTK_WINDOW(win));
    gtk_window_set_title(GTK_WINDOW(obj), msg);

    g_signal_connect(G_OBJECT(obj),
		     "response",
		     G_CALLBACK(trg_torrent_move_response_cb), win);

    return TRG_TORRENT_MOVE_DIALOG(obj);
}
