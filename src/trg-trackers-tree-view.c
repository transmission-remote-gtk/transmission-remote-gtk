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

G_DEFINE_TYPE(TrgTrackersTreeView, trg_trackers_tree_view,
	      TRG_TYPE_TREE_VIEW)

#define TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TRACKERS_TREE_VIEW, TrgTrackersTreeViewPrivate))

typedef struct _TrgTrackersTreeViewPrivate TrgTrackersTreeViewPrivate;

struct _TrgTrackersTreeViewPrivate {
    trg_client *client;
    GtkCellRenderer *announceRenderer;
};

static void
trg_trackers_tree_view_class_init(TrgTrackersTreeViewClass *
				  klass G_GNUC_UNUSED)
{
	g_type_class_add_private(klass, sizeof(TrgTrackersTreeViewPrivate));
}

void trg_trackers_tree_view_new_connection(TrgTrackersTreeView *tv, trg_client *tc)
{
	TrgTrackersTreeViewPrivate *priv = TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(tv);
	gboolean editable = tc->version >= 2.10;

	g_object_set(priv->announceRenderer, "editable", editable, NULL);
	g_object_set(priv->announceRenderer, "mode", editable ? GTK_CELL_RENDERER_MODE_EDITABLE : GTK_CELL_RENDERER_MODE_INERT, NULL);
}

static void trg_tracker_announce_edited(GtkCellRendererText *renderer,
                                                        gchar               *path,
                                                        gchar               *new_text,
                                                        gpointer             user_data)
{
	GtkTreeView *tv = GTK_TREE_VIEW(user_data);
	GtkTreeModel *model = gtk_tree_view_get_model(tv);
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TRACKERCOL_ANNOUNCE, new_text, -1);
}

static void trg_trackers_tree_view_init(TrgTrackersTreeView * self)
{
	TrgTrackersTreeViewPrivate *priv = TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(self);

    trg_tree_view_add_pixbuf_text_column(TRG_TREE_VIEW(self),
					 TRACKERCOL_ICON,
					 TRACKERCOL_TIER, "Tier", -1);

    priv->announceRenderer = trg_tree_view_add_column(TRG_TREE_VIEW(self), "Announce URL",
			     TRACKERCOL_ANNOUNCE);
    g_signal_connect(priv->announceRenderer, "edited", G_CALLBACK(trg_tracker_announce_edited), self);

    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Scrape URL",
			     TRACKERCOL_SCRAPE);
}

static void add_tracker(GtkWidget *w, gpointer data)
{

}

static void delete_tracker(GtkWidget *w, gpointer data)
{

}

static void
view_popup_menu_add_only(GtkWidget * treeview, GdkEventButton * event,
		gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem = trg_menu_bar_item_new(GTK_MENU_SHELL(menu), "Add", GTK_STOCK_ADD, TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_tracker), treeview);

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

    menuitem = trg_menu_bar_item_new(GTK_MENU_SHELL(menu), "Delete", GTK_STOCK_DELETE, TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(delete_tracker), treeview);

    menuitem = trg_menu_bar_item_new(GTK_MENU_SHELL(menu), "Add", GTK_STOCK_ADD, TRUE);
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_tracker), treeview);

    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		   (event != NULL) ? event->button : 0,
		   gdk_event_get_time((GdkEvent *) event));
}

static gboolean
view_onButtonPressed(GtkWidget * treeview, GdkEventButton * event,
		     gpointer userdata)
{
	TrgTrackersTreeViewPrivate *priv = TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(treeview);
    GtkTreeSelection *selection;
    GtkTreePath *path;

    if (priv->client->version < 2.10)
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
	    return TRUE;
	} else {
    	view_popup_menu_add_only(treeview, event, userdata);
    }
    }

    return FALSE;
}

static gboolean view_onPopupMenu(GtkWidget * treeview, gpointer userdata)
{
    view_popup_menu(treeview, NULL, userdata);
    return TRUE;
}

TrgTrackersTreeView *trg_trackers_tree_view_new(TrgTrackersModel * model, trg_client *client)
{
    GObject *obj = g_object_new(TRG_TYPE_TRACKERS_TREE_VIEW, NULL);
    TrgTrackersTreeViewPrivate *priv = TRG_TRACKERS_TREE_VIEW_GET_PRIVATE(obj);

    g_signal_connect(obj, "button-press-event",
		     G_CALLBACK(view_onButtonPressed), NULL);
    g_signal_connect(obj, "popup-menu", G_CALLBACK(view_onPopupMenu),
		     NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));
    priv->client = client;

    return TRG_TRACKERS_TREE_VIEW(obj);
}
