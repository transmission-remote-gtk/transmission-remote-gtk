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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "torrent.h"
#include "json.h"
#include "trg-client.h"
#include "trg-json-widgets.h"
#include "requests.h"
#include "protocol-constants.h"
#include "trg-torrent-model.h"
#include "trg-torrent-tree-view.h"
#include "trg-torrent-props-dialog.h"
#include "trg-main-window.h"
#include "hig.h"

G_DEFINE_TYPE(TrgTorrentPropsDialog, trg_torrent_props_dialog,
              GTK_TYPE_DIALOG)

enum {
    PROP_0,
    PROP_TREEVIEW,
    PROP_PARENT_WINDOW,
    PROP_CLIENT
};

#define TRG_TORRENT_PROPS_DIALOG_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_PROPS_DIALOG, TrgTorrentPropsDialogPrivate))
typedef struct _TrgTorrentPropsDialogPrivate TrgTorrentPropsDialogPrivate;

struct _TrgTorrentPropsDialogPrivate {
    TrgTorrentTreeView *tv;
    TrgClient *client;
    TrgMainWindow *parent;
    JsonArray *targetIds;

    GList *widgets;

    GtkWidget *bandwidthPriorityCombo, *seedRatioMode;
};

static void
trg_torrent_props_dialog_set_property(GObject * object,
                                      guint prop_id,
                                      const GValue * value,
                                      GParamSpec * pspec G_GNUC_UNUSED)
{
    TrgTorrentPropsDialogPrivate *priv =
        TRG_TORRENT_PROPS_DIALOG_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_PARENT_WINDOW:
        priv->parent = g_value_get_object(value);
        break;
    case PROP_TREEVIEW:
        priv->tv = g_value_get_object(value);
        break;
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    }
}

static void
trg_torrent_props_dialog_get_property(GObject * object,
                                      guint prop_id,
                                      GValue * value,
                                      GParamSpec * pspec G_GNUC_UNUSED)
{
    TrgTorrentPropsDialogPrivate *priv =
        TRG_TORRENT_PROPS_DIALOG_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_TREEVIEW:
        g_value_set_object(value, priv->tv);
        break;
    case PROP_PARENT_WINDOW:
        g_value_set_object(value, priv->parent);
        break;
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    }
}

static void
trg_torrent_props_response_cb(GtkDialog * dlg, gint res_id,
                              gpointer data G_GNUC_UNUSED)
{
    TrgTorrentPropsDialogPrivate *priv;
    JsonNode *request;
    JsonObject *args;

    priv = TRG_TORRENT_PROPS_DIALOG_GET_PRIVATE(dlg);

    if (res_id != GTK_RESPONSE_OK) {
        gtk_widget_destroy(GTK_WIDGET(dlg));
        return;
    }

    request = torrent_set(priv->targetIds);
    request_set_tag_from_ids(request, priv->targetIds);
    args = node_get_arguments(request);

    json_object_set_int_member(args, FIELD_SEED_RATIO_MODE,
                               gtk_combo_box_get_active(GTK_COMBO_BOX
                                                        (priv->
                                                         seedRatioMode)));
    json_object_set_int_member(args, FIELD_BANDWIDTH_PRIORITY,
                               gtk_combo_box_get_active(GTK_COMBO_BOX
                                                        (priv->
                                                         bandwidthPriorityCombo)) - 1);

    trg_json_widgets_save(priv->widgets, args);
    trg_json_widget_desc_list_free(priv->widgets);

    dispatch_async(priv->client, request,
                   on_generic_interactive_action, priv->parent);

    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void seed_ratio_mode_changed_cb(GtkWidget * w, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_combo_box_get_active(GTK_COMBO_BOX(w))
                             == 1 ? TRUE : FALSE);
}

static GtkWidget *trg_props_limitsPage(TrgTorrentPropsDialog * win,
                                       JsonObject * json)
{
    TrgTorrentPropsDialogPrivate *priv =
        TRG_TORRENT_PROPS_DIALOG_GET_PRIVATE(win);
    GtkWidget *w, *tb, *t;
    gint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Bandwidth"));

    w = trg_json_widget_check_new(&priv->widgets, json, FIELD_HONORS_SESSION_LIMITS, _("Honor global limits"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = priv->bandwidthPriorityCombo = gtk_combo_box_new_text();
    //widget_set_json_key(w, FIELD_BANDWIDTH_PRIORITY);
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("Low"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("Normal"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("High"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w),
                             torrent_get_bandwidth_priority(json) + 1);

    hig_workarea_add_row(t, &row, _("Torrent priority:"), w, NULL);

    if (json_object_has_member(json, FIELD_QUEUE_POSITION)) {
        w = trg_json_widget_spin_new_int(&priv->widgets, json, FIELD_QUEUE_POSITION, NULL, 0, INT_MAX, 1);
        hig_workarea_add_row(t, &row, _("Queue Position:"), w, w);
    }

    tb = trg_json_widget_check_new(&priv->widgets, json, FIELD_DOWNLOAD_LIMITED, _("Limit download speed (Kbps)"), NULL);
    w = trg_json_widget_spin_new_int(&priv->widgets, json, FIELD_DOWNLOAD_LIMIT, tb, 0, INT_MAX, 1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    tb = trg_json_widget_check_new(&priv->widgets, json, FIELD_UPLOAD_LIMITED, _("Limit upload speed (Kbps)"), NULL);
    w = trg_json_widget_spin_new_int(&priv->widgets, json, FIELD_UPLOAD_LIMIT, tb, 0, INT_MAX, 1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    hig_workarea_add_section_title(t, &row, _("Seeding"));

    w = priv->seedRatioMode = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("Use global settings"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(w),
                              _("Stop seeding at ratio"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(w),
                              _("Seed regardless of ratio"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w),
                             torrent_get_seed_ratio_mode(json));
    hig_workarea_add_row(t, &row, _("Seed ratio mode:"), w, NULL);

    w = trg_json_widget_spin_new_double(&priv->widgets, json, FIELD_SEED_RATIO_LIMIT, NULL, 0, INT_MAX, 0.2);
    seed_ratio_mode_changed_cb(priv->seedRatioMode, w);
    g_signal_connect(G_OBJECT(priv->seedRatioMode), "changed",
                     G_CALLBACK(seed_ratio_mode_changed_cb), w);
    hig_workarea_add_row(t, &row, _("Seed ratio limit:"), w, w);

    hig_workarea_add_section_title(t, &row, _("Peers"));

    w = trg_json_widget_spin_new_int(&priv->widgets, json, FIELD_PEER_LIMIT, NULL, 0, INT_MAX, 5);
    hig_workarea_add_row(t, &row, _("Peer limit:"), w, w);

    return t;
}

static GObject *trg_torrent_props_dialog_constructor(GType type,
                                                     guint
                                                     n_construct_properties,
                                                     GObjectConstructParam
                                                     * construct_params)
{
    GObject *object;
    TrgTorrentPropsDialogPrivate *priv;
    JsonObject *json;
    GtkTreeSelection *selection;
    gint rowCount;
    GtkWidget *notebook;

    object = G_OBJECT_CLASS
        (trg_torrent_props_dialog_parent_class)->constructor(type,
                                                             n_construct_properties,
                                                             construct_params);

    priv = TRG_TORRENT_PROPS_DIALOG_GET_PRIVATE(object);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->tv));
    rowCount = gtk_tree_selection_count_selected_rows(selection);
    get_torrent_data(trg_client_get_torrent_table(priv->client),
                     trg_mw_get_selected_torrent_id(priv->parent), &json,
                     NULL);
    priv->targetIds = build_json_id_array(priv->tv);

    if (rowCount > 1) {
        gchar *windowTitle =
            g_strdup_printf(_("Multiple (%d) torrent properties"),
                            rowCount);
        gtk_window_set_title(GTK_WINDOW(object), windowTitle);
        g_free(windowTitle);
    } else if (rowCount == 1) {
        gtk_window_set_title(GTK_WINDOW(object), torrent_get_name(json));
    }

    gtk_window_set_transient_for(GTK_WINDOW(object),
                                 GTK_WINDOW(priv->parent));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(object), TRUE);

    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_CLOSE,
                          GTK_RESPONSE_CLOSE);
    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_OK,
                          GTK_RESPONSE_OK);

    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(object),
                                    GTK_RESPONSE_OK);

    gtk_dialog_set_alternative_button_order(GTK_DIALOG(object),
                                            GTK_RESPONSE_ACCEPT,
                                            GTK_RESPONSE_OK, -1);

    g_signal_connect(G_OBJECT(object),
                     "response",
                     G_CALLBACK(trg_torrent_props_response_cb), NULL);

    notebook = gtk_notebook_new();

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_props_limitsPage
                             (TRG_TORRENT_PROPS_DIALOG(object), json),
                             gtk_label_new(_("Limits")));

    gtk_container_set_border_width(GTK_CONTAINER(notebook), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(object)->vbox), notebook,
                       TRUE, TRUE, 0);

    return object;
}

static void
trg_torrent_props_dialog_class_init(TrgTorrentPropsDialogClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructor = trg_torrent_props_dialog_constructor;
    object_class->set_property = trg_torrent_props_dialog_set_property;
    object_class->get_property = trg_torrent_props_dialog_get_property;

    g_type_class_add_private(klass, sizeof(TrgTorrentPropsDialogPrivate));

    g_object_class_install_property(object_class,
                                    PROP_TREEVIEW,
                                    g_param_spec_object
                                    ("torrent-tree-view",
                                     "TrgTorrentTreeView",
                                     "TrgTorrentTreeView",
                                     TRG_TYPE_TORRENT_TREE_VIEW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_PARENT_WINDOW,
                                    g_param_spec_object
                                    ("parent-window", "Parent window",
                                     "Parent window", TRG_TYPE_MAIN_WINDOW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_CLIENT,
                                    g_param_spec_pointer
                                    ("trg-client", "TClient",
                                     "Client",
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));
}

static void
trg_torrent_props_dialog_init(TrgTorrentPropsDialog * self G_GNUC_UNUSED)
{

}

TrgTorrentPropsDialog *trg_torrent_props_dialog_new(GtkWindow * window,
                                                    TrgTorrentTreeView *
                                                    treeview,
                                                    TrgClient * client)
{
    return g_object_new(TRG_TYPE_TORRENT_PROPS_DIALOG,
                        "parent-window", window,
                        "torrent-tree-view", treeview,
                        "trg-client", client, NULL);
}
