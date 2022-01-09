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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-prefs.h"
#include "trg-trackers-tree-view.h"
#include "trg-tree-view.h"
#include "trg-client.h"
#include "trg-menu-bar.h"
#include "requests.h"
#include "json.h"
#include "trg-trackers-model.h"
#include "trg-main-window.h"

G_DEFINE_TYPE(TrgTrackersTreeView, trg_trackers_tree_view,
              TRG_TYPE_TREE_VIEW)
#define TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TRACKERS_TREE_VIEW, TrgTrackersTreeViewPrivate))
typedef struct _TrgTrackersTreeViewPrivate TrgTrackersTreeViewPrivate;

struct _TrgTrackersTreeViewPrivate {
    TrgClient *client;
    GtkCellRenderer *announceRenderer;
    GtkTreeViewColumn *announceColumn;
    TrgMainWindow *win;
};

static void
trg_trackers_tree_view_class_init(TrgTrackersTreeViewClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgTrackersTreeViewPrivate));
}

static gboolean is_tracker_edit_supported(TrgClient * tc)
{
    return trg_client_get_rpc_version(tc) >= 10;
}

static gboolean on_trackers_update(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(response->cb_data);
    GtkTreeModel *model =
        gtk_tree_view_get_model(GTK_TREE_VIEW(response->cb_data));

    trg_trackers_model_set_accept(TRG_TRACKERS_MODEL(model), TRUE);

    response->cb_data = priv->win;
    return on_generic_interactive_action_response(data);
}

void
trg_trackers_tree_view_new_connection(TrgTrackersTreeView * tv,
                                      TrgClient * tc)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(tv);

    gboolean editable = is_tracker_edit_supported(tc);

    g_object_set(priv->announceRenderer,
                 "editable", editable,
                 "mode", editable ? GTK_CELL_RENDERER_MODE_EDITABLE :
                 GTK_CELL_RENDERER_MODE_INERT, NULL);
}

static void
trg_tracker_announce_edited(GtkCellRendererText * renderer,
                            gchar * path,
                            gchar * new_text, gpointer user_data)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(user_data);
    GtkTreeModel *model =
        gtk_tree_view_get_model(GTK_TREE_VIEW(user_data));
    gint64 torrentId =
        trg_trackers_model_get_torrent_id(TRG_TRACKERS_MODEL(model));
    JsonArray *torrentIds = json_array_new();
    JsonArray *trackerModifiers = json_array_new();

    gint64 trackerId;
    JsonNode *req;
    JsonObject *args;
    GtkTreeIter iter;
    gchar *icon;

    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, TRACKERCOL_ANNOUNCE,
                       new_text, -1);
    gtk_tree_model_get(model, &iter, TRACKERCOL_ID, &trackerId,
                       TRACKERCOL_ICON, &icon, -1);

    json_array_add_int_element(torrentIds, torrentId);

    req = torrent_set(torrentIds);
    args = node_get_arguments(req);

    if (!g_strcmp0(icon, "list-add")) {
        json_array_add_string_element(trackerModifiers, new_text);
        json_object_set_array_member(args, "trackerAdd", trackerModifiers);
    } else {
        json_array_add_int_element(trackerModifiers, trackerId);
        json_array_add_string_element(trackerModifiers, new_text);
        json_object_set_array_member(args, "trackerReplace",
                                     trackerModifiers);
    }

    g_free(icon);

    dispatch_async(priv->client, req, on_trackers_update, user_data);
}

static void
trg_tracker_announce_editing_started(GtkCellRenderer *
                                     renderer G_GNUC_UNUSED,
                                     GtkCellEditable *
                                     editable G_GNUC_UNUSED,
                                     gchar *
                                     path G_GNUC_UNUSED,
                                     gpointer user_data)
{
    TrgTrackersModel *model =
        TRG_TRACKERS_MODEL(gtk_tree_view_get_model
                           (GTK_TREE_VIEW(user_data)));

    trg_trackers_model_set_accept(model, FALSE);
}

static void
trg_tracker_announce_editing_canceled(GtkWidget *
                                      w G_GNUC_UNUSED, gpointer data)
{
    TrgTrackersModel *model =
        TRG_TRACKERS_MODEL(gtk_tree_view_get_model(GTK_TREE_VIEW(data)));

    trg_trackers_model_set_accept(model, TRUE);
}

static void trg_trackers_tree_view_init(TrgTrackersTreeView * self)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(self);
    TrgTreeView *ttv = TRG_TREE_VIEW(self);
    trg_column_description *desc;

    desc =
        trg_tree_view_reg_column(ttv, TRG_COLTYPE_ICONTEXT,
                                 TRACKERCOL_TIER, _("Tier"), "tier",
                                 TRG_COLUMN_UNREMOVABLE);
    desc->model_column_extra = TRACKERCOL_ICON;

    desc =
        trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT,
                                 TRACKERCOL_ANNOUNCE, _("Announce URL"),
                                 "announce-url", TRG_COLUMN_UNREMOVABLE);
    priv->announceRenderer = desc->customRenderer =
        gtk_cell_renderer_text_new();
    g_signal_connect(priv->announceRenderer, "edited",
                     G_CALLBACK(trg_tracker_announce_edited), self);
    g_signal_connect(priv->announceRenderer, "editing-canceled",
                     G_CALLBACK(trg_tracker_announce_editing_canceled),
                     self);
    g_signal_connect(priv->announceRenderer, "editing-started",
                     G_CALLBACK(trg_tracker_announce_editing_started),
                     self);
    desc->out = &priv->announceColumn;

    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TRACKERCOL_LAST_ANNOUNCE_PEER_COUNT,
                             _("Peers"), "last-announce-peer-count", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TRACKERCOL_SEEDERCOUNT, _("Seeder Count"),
                             "seeder-count", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TRACKERCOL_LEECHERCOUNT, _("Leecher Count"),
                             "leecher-count", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_EPOCH,
                             TRACKERCOL_LAST_ANNOUNCE_TIME,
                             _("Last Announce"), "last-announce-time", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT,
                             TRACKERCOL_LAST_ANNOUNCE_RESULT,
                             _("Last Result"), "last-result", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, TRACKERCOL_SCRAPE,
                             _("Scrape URL"), "scrape-url", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_EPOCH,
                             TRACKERCOL_LAST_SCRAPE_TIME, _("Last Scrape"),
                             "last-scrape-time", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, TRACKERCOL_HOST,
                             _("Host"), "host", TRG_COLUMN_EXTRA);
}

static void add_tracker(GtkWidget * w, gpointer data)
{
    GtkTreeView *tv = GTK_TREE_VIEW(data);
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(data);
    GtkTreeModel *model = gtk_tree_view_get_model(tv);
    GtkTreeIter iter;
    GtkTreePath *path;

    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, TRACKERCOL_ICON,
                       "list-add", -1);

    path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_view_set_cursor(tv, path, priv->announceColumn, TRUE);
    gtk_tree_path_free(path);
}

static void delete_tracker(GtkWidget * w, gpointer data)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(data);
    GtkTreeView *tv = GTK_TREE_VIEW(data);
    GList *selectionRefs = trg_tree_view_get_selected_refs_list(tv);
    GtkTreeModel *model = gtk_tree_view_get_model(tv);
    JsonArray *trackerIds = json_array_new();
    gint64 torrentId =
        trg_trackers_model_get_torrent_id(TRG_TRACKERS_MODEL(model));
    JsonArray *torrentIds = json_array_new();

    JsonNode *req;
    JsonObject *args;
    GList *li;

    for (li = selectionRefs; li; li = g_list_next(li)) {
        GtkTreeRowReference *rr = (GtkTreeRowReference *) li->data;
        GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
        if (path) {
            gint64 trackerId;
            GtkTreeIter trackerIter;
            gtk_tree_model_get_iter(model, &trackerIter, path);
            gtk_tree_model_get(model, &trackerIter, TRACKERCOL_ID,
                               &trackerId, -1);
            json_array_add_int_element(trackerIds, trackerId);
            gtk_list_store_remove(GTK_LIST_STORE(model), &trackerIter);
            gtk_tree_path_free(path);
        }
        gtk_tree_row_reference_free(rr);
    }
    g_list_free(selectionRefs);

    json_array_add_int_element(torrentIds, torrentId);

    req = torrent_set(torrentIds);
    args = node_get_arguments(req);

    json_object_set_array_member(args, "trackerRemove", trackerIds);

    trg_trackers_model_set_accept(TRG_TRACKERS_MODEL(model), FALSE);

    dispatch_async(priv->client, req, on_trackers_update, data);
}

static void
view_popup_menu_add_only(GtkWidget * treeview, GdkEventButton * event,
                         gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem =
        trg_menu_bar_item_new(GTK_MENU_SHELL(menu), _("Add"),
                              TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_tracker),
                     treeview);

    gtk_widget_show_all(menu);

    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
}

static void
view_popup_menu(GtkWidget * treeview, GdkEventButton * event,
                gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem =
        trg_menu_bar_item_new(GTK_MENU_SHELL(menu), _("Delete"),
                              TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(delete_tracker),
                     treeview);

    menuitem =
        trg_menu_bar_item_new(GTK_MENU_SHELL(menu), _("Add"),
                              TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_tracker),
                     treeview);

    gtk_widget_show_all(menu);

    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
}

static gboolean
view_onButtonPressed(GtkWidget * treeview, GdkEventButton * event,
                     gpointer userdata)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(treeview);
    TrgTrackersModel *model =
        TRG_TRACKERS_MODEL(gtk_tree_view_get_model
                           (GTK_TREE_VIEW(treeview)));
    GtkTreeSelection *selection;
    GtkTreePath *path;

    if (!is_tracker_edit_supported(priv->client))
        return FALSE;

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                          (gint) event->x,
                                          (gint) event->y, &path,
                                          NULL, NULL, NULL)) {
            if (!gtk_tree_selection_path_is_selected(selection, path)) {
                gtk_tree_selection_unselect_all(selection);
                gtk_tree_selection_select_path(selection, path);
            }
            gtk_tree_path_free(path);

            view_popup_menu(treeview, event, userdata);
        } else if (trg_trackers_model_get_torrent_id(model) >= 0) {
            view_popup_menu_add_only(treeview, event, userdata);
        }
        return TRUE;
    }

    return FALSE;
}

static gboolean view_onPopupMenu(GtkWidget * treeview, gpointer userdata)
{
    view_popup_menu(treeview, NULL, userdata);
    return TRUE;
}

TrgTrackersTreeView *trg_trackers_tree_view_new(TrgTrackersModel * model,
                                                TrgClient * client,
                                                TrgMainWindow * win,
                                                const gchar * configId)
{
    GObject *obj = g_object_new(TRG_TYPE_TRACKERS_TREE_VIEW,
                                "config-id", configId,
                                "prefs", trg_client_get_prefs(client),
                                NULL);

    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(obj);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));
    priv->client = client;
    priv->win = win;

    trg_tree_view_setup_columns(TRG_TREE_VIEW(obj));

    g_signal_connect(obj, "button-press-event",
                     G_CALLBACK(view_onButtonPressed), NULL);
    g_signal_connect(obj, "popup-menu", G_CALLBACK(view_onPopupMenu),
                     NULL);

    return TRG_TRACKERS_TREE_VIEW(obj);
}
