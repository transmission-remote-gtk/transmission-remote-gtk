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

#include <glib-object.h>
#include <gtk/gtk.h>

#include "torrent.h"
#include "trg-state-selector.h"

enum {
    SELECTOR_STATE_CHANGED,
    SELECTOR_SIGNAL_COUNT
};

static guint signals[SELECTOR_SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(TrgStateSelector, trg_state_selector, GTK_TYPE_TREE_VIEW)
#define TRG_STATE_SELECTOR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_STATE_SELECTOR, TrgStateSelectorPrivate))
typedef struct _TrgStateSelectorPrivate TrgStateSelectorPrivate;

struct _TrgStateSelectorPrivate {
    guint flag;
};

guint32 trg_state_selector_get_flag(TrgStateSelector * s)
{
    TrgStateSelectorPrivate *priv = TRG_STATE_SELECTOR_GET_PRIVATE(s);
    return priv->flag;
}

static void trg_state_selector_class_init(TrgStateSelectorClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    signals[SELECTOR_STATE_CHANGED] =
	g_signal_new("torrent-state-changed",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		     G_STRUCT_OFFSET(TrgStateSelectorClass,
				     torrent_state_changed), NULL,
		     NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE,
		     1, G_TYPE_UINT);

    g_type_class_add_private(klass, sizeof(TrgStateSelectorPrivate));
}

static void state_selection_changed(GtkTreeSelection * selection,
				    gpointer data)
{
    TrgStateSelectorPrivate *priv;
    GtkTreeIter iter;
    GtkTreeView *tv;
    GtkTreeModel *stateModel;

    priv = TRG_STATE_SELECTOR_GET_PRIVATE(data);

    tv = gtk_tree_selection_get_tree_view(selection);
    stateModel = gtk_tree_view_get_model(tv);

    if (gtk_tree_selection_get_selected(selection, &stateModel, &iter))
	gtk_tree_model_get(stateModel, &iter, STATE_SELECTOR_BIT,
			   &(priv->flag), -1);
    else
	priv->flag = 0;

    g_signal_emit(TRG_STATE_SELECTOR(data),
		  signals[SELECTOR_STATE_CHANGED], 0, priv->flag);
}

static void trg_state_selector_add_state(GtkListStore * model,
					 GtkTreeIter * iter, gchar * icon,
					 gchar * name, guint32 flag)
{
    gtk_list_store_append(model, iter);
    gtk_list_store_set(model, iter,
		       STATE_SELECTOR_ICON, icon,
		       STATE_SELECTOR_NAME, name,
		       STATE_SELECTOR_BIT, flag, -1);
}

static void trg_state_selector_init(TrgStateSelector * self)
{
    TrgStateSelectorPrivate *priv;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreeSelection *selection;

    priv = TRG_STATE_SELECTOR_GET_PRIVATE(self);
    priv->flag = 0;

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);

    column = gtk_tree_view_column_new();

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    g_object_set(renderer, "stock-size", 4, NULL);
    gtk_tree_view_column_set_attributes(column, renderer, "stock-id",
					0, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);

    store =
	gtk_list_store_new(STATE_SELECTOR_COLUMNS, G_TYPE_STRING,
			   G_TYPE_STRING, G_TYPE_UINT);

    trg_state_selector_add_state(store, &iter, GTK_STOCK_ABOUT, "All", 0);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_GO_DOWN,
				 "Downloading", TORRENT_FLAG_DOWNLOADING);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_MEDIA_PAUSE,
				 "Paused", TORRENT_FLAG_PAUSED);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_REFRESH,
				 "Checking", TORRENT_FLAG_CHECKING);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_APPLY,
				 "Complete", TORRENT_FLAG_COMPLETE);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_SELECT_ALL,
				 "Incomplete", TORRENT_FLAG_INCOMPLETE);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_GO_UP,
				 "Seeding", TORRENT_FLAG_SEEDING);
    trg_state_selector_add_state(store, &iter,
				 GTK_STOCK_DIALOG_WARNING, "Error",
				 TORRENT_FLAG_ERROR);

    gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(store));
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(self), TRUE);

    gtk_widget_set_size_request(GTK_WIDGET(self), 120, -1);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self));

    g_signal_connect(G_OBJECT(selection), "changed",
		     G_CALLBACK(state_selection_changed), self);
}

TrgStateSelector *trg_state_selector_new(void)
{
    return g_object_new(TRG_TYPE_STATE_SELECTOR, NULL);
}
