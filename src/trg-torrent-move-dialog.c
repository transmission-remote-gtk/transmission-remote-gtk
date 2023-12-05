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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <string.h>

#include "hig.h"
#include "requests.h"
#include "torrent.h"
#include "trg-client.h"
#include "trg-destination-combo.h"
#include "trg-main-window.h"
#include "trg-torrent-move-dialog.h"

struct _TrgTorrentMoveDialog {
    GtkDialog parent;

    TrgClient *client;
    TrgMainWindow *win;
    TrgTorrentTreeView *treeview;
    JsonArray *ids;
    GtkWidget *location_combo;
    GtkWidget *move_check;
    GtkWidget *move_button;
};

G_DEFINE_TYPE(TrgTorrentMoveDialog, trg_torrent_move_dialog, GTK_TYPE_DIALOG)

enum {
    PROP_0,
    PROP_CLIENT,
    PROP_PARENT_WINDOW,
    PROP_TREEVIEW
};

static void trg_torrent_move_response_cb(GtkDialog *dialog, gint res_id, gpointer data)
{
    TrgTorrentMoveDialog *dlg = TRG_TORRENT_MOVE_DIALOG(dialog);

    if (res_id == GTK_RESPONSE_ACCEPT) {
        gchar *location = trg_destination_combo_get_dir(TRG_DESTINATION_COMBO(dlg->location_combo));
        JsonNode *request = torrent_set_location(
            dlg->ids, location, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->move_check)));
        g_free(location);
        trg_destination_combo_save_selection(TRG_DESTINATION_COMBO(dlg->location_combo));
        dispatch_rpc_async(dlg->client, request, on_generic_interactive_action_response, data);
    } else {
        json_array_unref(dlg->ids);
    }
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void location_changed(GtkComboBox *w, gpointer data)
{
    TrgTorrentMoveDialog *dlg = TRG_TORRENT_MOVE_DIALOG(data);
    gtk_widget_set_sensitive(
        dlg->move_button,
        trg_destination_combo_has_text(TRG_DESTINATION_COMBO(dlg->location_combo)));
}

static GObject *trg_torrent_move_dialog_constructor(GType type, guint n_construct_properties,
                                                    GObjectConstructParam *construct_params)
{
    GObject *object = G_OBJECT_CLASS(trg_torrent_move_dialog_parent_class)
                          ->constructor(type, n_construct_properties, construct_params);
    TrgTorrentMoveDialog *self = TRG_TORRENT_MOVE_DIALOG(object);

    gint count;
    gchar *msg;

    GtkWidget *w, *t;
    guint row = 0;

    t = hig_workarea_create();

    w = self->location_combo
        = trg_destination_combo_new(self->client, TRG_PREFS_KEY_LAST_MOVE_DESTINATION);
    g_signal_connect(w, "changed", G_CALLBACK(location_changed), object);
    hig_workarea_add_row(t, &row, _("Location:"), w, NULL);

    self->move_check = hig_workarea_add_wide_checkbutton(t, &row, _("Move"), TRUE);

    gtk_window_set_destroy_with_parent(GTK_WINDOW(object), TRUE);

    gtk_dialog_add_button(GTK_DIALOG(object), _("_Close"), GTK_RESPONSE_CANCEL);
    self->move_button = gtk_dialog_add_button(GTK_DIALOG(object), _("Move"), GTK_RESPONSE_ACCEPT);

    gtk_widget_set_sensitive(
        self->move_button,
        trg_destination_combo_has_text(TRG_DESTINATION_COMBO(self->location_combo)));

    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(object), GTK_RESPONSE_ACCEPT);

    gtk_container_set_border_width(GTK_CONTAINER(t), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(object))), t, TRUE, TRUE, 0);

    count = gtk_tree_selection_count_selected_rows(
        gtk_tree_view_get_selection(GTK_TREE_VIEW(self->treeview)));
    self->ids = build_json_id_array(self->treeview);

    if (count == 1) {
        JsonObject *json;
        const gchar *name;

        get_torrent_data(trg_client_get_torrent_table(self->client),
                         trg_mw_get_selected_torrent_id(self->win), &json, NULL);
        name = torrent_get_name(json);
        msg = g_strdup_printf(_("Move %s"), name);
    } else {
        msg = g_strdup_printf(_("Move %d torrents"), count);
    }

    gtk_window_set_transient_for(GTK_WINDOW(object), GTK_WINDOW(self->win));
    gtk_window_set_title(GTK_WINDOW(object), msg);

    g_signal_connect(G_OBJECT(object), "response", G_CALLBACK(trg_torrent_move_response_cb),
                     self->win);

    g_free(msg);

    return object;
}

static void trg_torrent_move_dialog_get_property(GObject *object, guint property_id, GValue *value,
                                                 GParamSpec *pspec)
{
    TrgTorrentMoveDialog *self = TRG_TORRENT_MOVE_DIALOG(object);
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, self->client);
        break;
    case PROP_PARENT_WINDOW:
        g_value_set_object(value, self->win);
        break;
    case PROP_TREEVIEW:
        g_value_set_object(value, self->treeview);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_torrent_move_dialog_set_property(GObject *object, guint property_id,
                                                 const GValue *value, GParamSpec *pspec)
{
    TrgTorrentMoveDialog *self = TRG_TORRENT_MOVE_DIALOG(object);
    switch (property_id) {
    case PROP_CLIENT:
        self->client = g_value_get_pointer(value);
        break;
    case PROP_PARENT_WINDOW:
        self->win = g_value_get_object(value);
        break;
    case PROP_TREEVIEW:
        self->treeview = g_value_get_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_torrent_move_dialog_class_init(TrgTorrentMoveDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_torrent_move_dialog_get_property;
    object_class->set_property = trg_torrent_move_dialog_set_property;
    object_class->constructor = trg_torrent_move_dialog_constructor;

    g_object_class_install_property(
        object_class, PROP_TREEVIEW,
        g_param_spec_object("torrent-tree-view", "TrgTorrentTreeView", "TrgTorrentTreeView",
                            TRG_TYPE_TORRENT_TREE_VIEW,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_PARENT_WINDOW,
        g_param_spec_object("parent-window", "Parent window", "Parent window", TRG_TYPE_MAIN_WINDOW,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_CLIENT,
        g_param_spec_pointer("trg-client", "TClient", "Client",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                 | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

static void trg_torrent_move_dialog_init(TrgTorrentMoveDialog *self)
{
}

TrgTorrentMoveDialog *trg_torrent_move_dialog_new(TrgMainWindow *win, TrgClient *client,
                                                  TrgTorrentTreeView *ttv)
{
    return g_object_new(TRG_TYPE_TORRENT_MOVE_DIALOG, "trg-client", client, "torrent-tree-view",
                        ttv, "parent-window", win, NULL);
}
