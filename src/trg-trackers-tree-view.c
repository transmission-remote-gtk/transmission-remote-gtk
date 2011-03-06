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

#include "trg-trackers-tree-view.h"
#include "trg-tree-view.h"
#include "trg-client.h"
#include "trg-menu-bar.h"
#include "requests.h"
#include "dispatch.h"
#include "json.h"
#include "trg-trackers-model.h"
#include "trg-main-window.h"

G_DEFINE_TYPE(TrgTrackersTreeView, trg_trackers_tree_view,
              TRG_TYPE_TREE_VIEW)
#define TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TRACKERS_TREE_VIEW, TrgTrackersTreeViewPrivate))
typedef struct _TrgTrackersTreeViewPrivate TrgTrackersTreeViewPrivate;

struct _TrgTrackersTreeViewPrivate {
    trg_client *client;
    GtkCellRenderer *announceRenderer;
    GtkTreeViewColumn *announceColumn;
    TrgMainWindow *win;
};

static void
trg_trackers_tree_view_class_init(TrgTrackersTreeViewClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgTrackersTreeViewPrivate));
}

static void
on_trackers_update(JsonObject * response, int status, gpointer data)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(data);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(data));

    trg_trackers_model_set_accept(TRG_TRACKERS_MODEL(model), TRUE);

    on_generic_interactive_action(response, status, priv->win);
}

void trg_trackers_tree_view_new_connection(TrgTrackersTreeView * tv,
                                           trg_client * tc)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(tv);
    gboolean editable = trg_client_supports_tracker_edit(tc);

    g_object_set(priv->announceRenderer, "editable", editable, NULL);
    g_object_set(priv->announceRenderer, "mode",
                 editable ? GTK_CELL_RENDERER_MODE_EDITABLE :
                 GTK_CELL_RENDERER_MODE_INERT, NULL);
}

static void trg_tracker_announce_edited(GtkCellRendererText * renderer,
                                        gchar * path,
                                        gchar * new_text,
                                        gpointer user_data)
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

    if (g_strcmp0(icon, GTK_STOCK_ADD) == 0) {
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

static void trg_tracker_announce_editing_started(GtkCellRenderer *
                                                 renderer,
                                                 GtkCellEditable *
                                                 editable, gchar * path,
                                                 gpointer user_data)
{
    TrgTrackersModel *model =
        TRG_TRACKERS_MODEL(gtk_tree_view_get_model
                           (GTK_TREE_VIEW(user_data)));

    trg_trackers_model_set_accept(model, FALSE);
}

static void trg_tracker_announce_editing_canceled(GtkWidget * w,
                                                  gpointer data)
{
    TrgTrackersModel *model =
        TRG_TRACKERS_MODEL(gtk_tree_view_get_model(GTK_TREE_VIEW(data)));

    trg_trackers_model_set_accept(model, TRUE);
}

static void trg_trackers_tree_view_init(TrgTrackersTreeView * self)
{
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(self);

    trg_tree_view_add_pixbuf_text_column(TRG_TREE_VIEW(self),
                                         TRACKERCOL_ICON,
                                         TRACKERCOL_TIER, "Tier", -1);

    priv->announceRenderer = gtk_cell_renderer_text_new();
    g_signal_connect(priv->announceRenderer, "edited",
                     G_CALLBACK(trg_tracker_announce_edited), self);
    g_signal_connect(priv->announceRenderer, "editing-canceled",
                     G_CALLBACK(trg_tracker_announce_editing_canceled),
                     self);
    g_signal_connect(priv->announceRenderer, "editing-started",
                     G_CALLBACK(trg_tracker_announce_editing_started),
                     self);

    priv->announceColumn =
        gtk_tree_view_column_new_with_attributes("Announce URL",
                                                 priv->announceRenderer,
                                                 "text",
                                                 TRACKERCOL_ANNOUNCE,
                                                 NULL);

    trg_tree_view_std_column_setup(priv->announceColumn,
                                   TRACKERCOL_ANNOUNCE, -1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), priv->announceColumn);

    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Scrape URL",
                             TRACKERCOL_SCRAPE);
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
                       GTK_STOCK_ADD, -1);

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
    JsonArray *torrentIds = json_array_new();

    JsonNode *req;
    JsonObject *args;
    GList *li;

    for (li = selectionRefs; li != NULL; li = g_list_next(li)) {
        GtkTreeRowReference *rr = (GtkTreeRowReference *) li->data;
        GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
        if (path != NULL) {
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

    json_array_add_int_element(torrentIds,
                               trg_trackers_model_get_torrent_id
                               (TRG_TRACKERS_MODEL(model)));

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
        trg_menu_bar_item_new(GTK_MENU_SHELL(menu), "Add", GTK_STOCK_ADD,
                              TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_tracker),
                     treeview);

    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent *) event));
}

static void
view_popup_menu(GtkWidget * treeview, GdkEventButton * event,
                gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem =
        trg_menu_bar_item_new(GTK_MENU_SHELL(menu), "Delete",
                              GTK_STOCK_DELETE, TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(delete_tracker),
                     treeview);

    menuitem =
        trg_menu_bar_item_new(GTK_MENU_SHELL(menu), "Add", GTK_STOCK_ADD,
                              TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_tracker),
                     treeview);

    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent *) event));
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

    if (!trg_client_supports_tracker_edit(priv->client))
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
                                                trg_client * client,
                                                TrgMainWindow * win)
{
    GObject *obj = g_object_new(TRG_TYPE_TRACKERS_TREE_VIEW, NULL);
    TrgTrackersTreeViewPrivate *priv =
        TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(obj);

    g_signal_connect(obj, "button-press-event",
                     G_CALLBACK(view_onButtonPressed), NULL);
    g_signal_connect(obj, "popup-menu", G_CALLBACK(view_onPopupMenu),
                     NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));
    priv->client = client;
    priv->win = win;

    return TRG_TRACKERS_TREE_VIEW(obj);
}
