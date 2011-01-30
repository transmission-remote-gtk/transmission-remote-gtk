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
#include "trg-torrent-add-url-dialog.h"
#include "hig.h"
#include "requests.h"
#include "dispatch.h"

G_DEFINE_TYPE(TrgTorrentAddUrlDialog, trg_torrent_add_url_dialog,
	      GTK_TYPE_DIALOG)
#define TRG_TORRENT_ADD_URL_DIALOG_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_ADD_URL_DIALOG, TrgTorrentAddUrlDialogPrivate))
typedef struct _TrgTorrentAddUrlDialogPrivate
 TrgTorrentAddUrlDialogPrivate;

struct _TrgTorrentAddUrlDialogPrivate {
    trg_client *client;
    TrgMainWindow *win;
    GtkWidget *urlEntry, *startCheck, *addButton;
};

static void
trg_torrent_add_url_dialog_class_init(TrgTorrentAddUrlDialogClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgTorrentAddUrlDialogPrivate));
}

static void
trg_torrent_add_url_response_cb(GtkDialog * dlg, gint res_id,
				gpointer data)
{
    TrgTorrentAddUrlDialogPrivate *priv =
	TRG_TORRENT_ADD_URL_DIALOG_GET_PRIVATE(dlg);
    if (res_id == GTK_RESPONSE_ACCEPT) {
	JsonNode *request =
	    torrent_add_url(gtk_entry_get_text(GTK_ENTRY(priv->urlEntry)),
			    gtk_toggle_button_get_active
			    (GTK_TOGGLE_BUTTON(priv->startCheck)));
	dispatch_async(priv->client, request,
		       on_generic_interactive_action, data);
    }
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void url_entry_changed(GtkWidget * w, gpointer data)
{
    TrgTorrentAddUrlDialogPrivate *priv =
	TRG_TORRENT_ADD_URL_DIALOG_GET_PRIVATE(data);
    gtk_widget_set_sensitive(priv->addButton,
			     gtk_entry_get_text_length(GTK_ENTRY(w)) > 0);
}

static void trg_torrent_add_url_dialog_init(TrgTorrentAddUrlDialog * self)
{
    TrgTorrentAddUrlDialogPrivate *priv =
	TRG_TORRENT_ADD_URL_DIALOG_GET_PRIVATE(self);
    GtkWidget *w, *t;
    gint row = 0;

    t = hig_workarea_create();

    w = priv->urlEntry = gtk_entry_new();
    g_signal_connect(w, "changed", G_CALLBACK(url_entry_changed), self);
    w = hig_workarea_add_row(t, &row, "URL:", w, NULL);

    priv->startCheck =
	hig_workarea_add_wide_checkbutton(t, &row, "Start Paused", FALSE);

    gtk_window_set_title(GTK_WINDOW(self), "Add torrent from URL");
    gtk_window_set_destroy_with_parent(GTK_WINDOW(self), TRUE);

    gtk_dialog_add_button(GTK_DIALOG(self), GTK_STOCK_CLOSE,
			  GTK_RESPONSE_CANCEL);
    priv->addButton =
	gtk_dialog_add_button(GTK_DIALOG(self), GTK_STOCK_ADD,
			      GTK_RESPONSE_ACCEPT);
    gtk_widget_set_sensitive(priv->addButton, FALSE);

    gtk_container_set_border_width(GTK_CONTAINER(self), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(self), GTK_RESPONSE_ACCEPT);

    gtk_dialog_set_alternative_button_order(GTK_DIALOG(self),
					    GTK_RESPONSE_ACCEPT,
					    GTK_RESPONSE_CANCEL, -1);

    gtk_container_set_border_width(GTK_CONTAINER(t), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(self)->vbox), t, TRUE, TRUE, 0);
}

TrgTorrentAddUrlDialog *trg_torrent_add_url_dialog_new(TrgMainWindow * win,
						       trg_client * client)
{
    GObject *obj = g_object_new(TRG_TYPE_TORRENT_ADD_URL_DIALOG, NULL);
    TrgTorrentAddUrlDialogPrivate *priv =
	TRG_TORRENT_ADD_URL_DIALOG_GET_PRIVATE(obj);

    priv->client = client;
    priv->win = win;

    gtk_window_set_transient_for(GTK_WINDOW(obj), GTK_WINDOW(win));
    g_signal_connect(G_OBJECT(obj),
		     "response",
		     G_CALLBACK(trg_torrent_add_url_response_cb), win);

    return TRG_TORRENT_ADD_URL_DIALOG(obj);
}
