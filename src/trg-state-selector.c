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
#include <json-glib/json-glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "torrent.h"
#include "trg-state-selector.h"
#include "util.h"

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
    GHashTable *trackers;
    GRegex *urlHostRegex;
};

GRegex *trg_state_selector_get_url_host_regex(TrgStateSelector *s)
{
    TrgStateSelectorPrivate *priv = TRG_STATE_SELECTOR_GET_PRIVATE(s);
    return priv->urlHostRegex;
}

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

static GtkTreeRowReference *quick_tree_ref_new(GtkTreeModel *model, GtkTreeIter *iter)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    GtkTreeRowReference *rr = gtk_tree_row_reference_new(model, path);
    gtk_tree_path_free(path);
    return rr;
}

struct cruft_remove_args {
    GHashTable *table;
    gint64 serial;
};

static gboolean trg_state_selector_remove_cruft(gpointer key, gpointer value, gpointer data)
{
    struct cruft_remove_args *args = (struct cruft_remove_args*)data;
    GtkTreeRowReference *rr = (GtkTreeRowReference*)value;
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    gboolean remove;

    GtkTreeIter iter;
    gint64 currentSerial;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, STATE_SELECTOR_SERIAL, &currentSerial, -1);

    remove = (args->serial != currentSerial);

    gtk_tree_path_free(path);

    return remove;
}

gchar *trg_state_selector_get_selected_text(TrgStateSelector *s)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(s));
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name = NULL;

    if (gtk_tree_selection_get_selected(sel, &model, &iter))
        gtk_tree_model_get(model, &iter, STATE_SELECTOR_NAME, &name, -1);

    return name;
}

void trg_state_selector_update_trackers(TrgStateSelector *s, JsonArray *torrents, gint64 serial)
{
    TrgStateSelectorPrivate *priv = TRG_STATE_SELECTOR_GET_PRIVATE(s);
    GtkTreeModel *model;
    GtkTreeIter iter;
    int i, j;
    struct cruft_remove_args cruft;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(s));

    for (i = 0; i < json_array_get_length(torrents); i++) {
        JsonObject *t = json_array_get_object_element(torrents, i);
        JsonArray *trackers = torrent_get_trackers(t);

        for (j = 0; j < json_array_get_length(trackers); j++) {
            JsonObject *tracker = json_array_get_object_element(trackers, j);
            const gchar *announceUrl = tracker_get_announce(tracker);
            gchar *announceHost = trg_uri_host_extract(priv->urlHostRegex, announceUrl);
            gpointer result;

            if (!announceHost)
                continue;

            result = g_hash_table_lookup(priv->trackers, announceHost);

            if (result)
            {
                GtkTreeRowReference *rr = (GtkTreeRowReference*)result;
                GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
                gtk_tree_model_get_iter(model, &iter, path);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                        STATE_SELECTOR_SERIAL, serial, -1);
                gtk_tree_path_free(path);
            } else {
                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                   STATE_SELECTOR_ICON, GTK_STOCK_NETWORK,
                                   STATE_SELECTOR_NAME, announceHost,
                                   STATE_SELECTOR_SERIAL, serial,
                                   STATE_SELECTOR_BIT, FILTER_FLAG_TRACKER, -1);
                g_hash_table_insert(priv->trackers, announceHost, quick_tree_ref_new(model, &iter));
            }
        }
    }

    cruft.serial = serial;
    cruft.table = priv->trackers;

    g_hash_table_foreach_remove(priv->trackers, trg_state_selector_remove_cruft, &cruft);
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

static void remove_row_ref_and_free(GtkTreeRowReference *rr)
{
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    GtkTreeIter iter;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    gtk_tree_path_free(path);
    gtk_tree_row_reference_free(rr);
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

    priv->urlHostRegex = trg_uri_host_regex_new();
    priv->trackers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)remove_row_ref_and_free);

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
                           G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT64);

    trg_state_selector_add_state(store, &iter, GTK_STOCK_ABOUT, _("All"),
                                 0);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_GO_DOWN,
                                 _("Downloading"),
                                 TORRENT_FLAG_DOWNLOADING);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_MEDIA_PAUSE,
                                 _("Paused"), TORRENT_FLAG_PAUSED);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_REFRESH,
                                 _("Checking"), TORRENT_FLAG_CHECKING);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_APPLY,
                                 _("Complete"), TORRENT_FLAG_COMPLETE);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_SELECT_ALL,
                                 _("Incomplete"), TORRENT_FLAG_INCOMPLETE);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_GO_UP,
                                 _("Seeding"), TORRENT_FLAG_SEEDING);
    trg_state_selector_add_state(store, &iter, GTK_STOCK_DIALOG_WARNING,
                                 _("Error"), TORRENT_FLAG_ERROR);
    trg_state_selector_add_state(store, &iter, NULL,
                                   NULL, 0);

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
