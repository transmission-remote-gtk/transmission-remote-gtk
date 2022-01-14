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

#include "trg-prefs.h"
#include "trg-persistent-tree-view.h"
#include "trg-preferences-dialog.h"
#include "util.h"

/* A config TreeView which can save/restore simple data into the TrgConf backend.
 * It's not actually a GtkTreeView, it's a GtKVBox which contains the buttons
 * to add/remove entries as well as the TreeView.
 */

G_DEFINE_TYPE(TrgPersistentTreeView, trg_persistent_tree_view,
              GTK_TYPE_BOX)
#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_PERSISTENT_TREE_VIEW, TrgPersistentTreeViewPrivate))
typedef struct _TrgPersistentTreeViewPrivate
 TrgPersistentTreeViewPrivate;

enum {
    PROP_0, PROP_PREFS, PROP_KEY, PROP_MODEL, PROP_CONF_FLAGS
};

struct _TrgPersistentTreeViewPrivate {
    TrgPrefs *prefs;
    gchar *key;
    GSList *columns;
    GtkTreeView *tv;
    JsonArray *ja;
    GtkWidget *delButton;
    GtkWidget *upButton;
    GtkWidget *downButton;
    trg_pref_widget_desc *wd;
    GtkTreeModel *model;
    trg_persistent_tree_view_column *addSelect;
    gint conf_flags;
};

static void selection_changed(TrgPersistentTreeView * ptv,
                              GtkTreeSelection * selection)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(ptv);
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        gtk_widget_set_sensitive(priv->upButton, gtk_tree_path_prev(path));
        gtk_widget_set_sensitive(priv->downButton,
                                 gtk_tree_model_iter_next(model, &iter));
        gtk_tree_path_free(path);
        gtk_widget_set_sensitive(priv->delButton, TRUE);
    } else {
        gtk_widget_set_sensitive(priv->delButton, FALSE);
        gtk_widget_set_sensitive(priv->upButton, FALSE);
        gtk_widget_set_sensitive(priv->downButton, FALSE);
    }
}

static void selection_changed_cb(GtkTreeSelection * selection,
                                 gpointer data)
{
    selection_changed(TRG_PERSISTENT_TREE_VIEW(data), selection);
}

static void
trg_persistent_tree_view_edit(GtkCellRendererText * renderer,
                              gchar * path, gchar * new_text,
                              gpointer user_data)
{
    trg_persistent_tree_view_column *cd =
        (trg_persistent_tree_view_column *) user_data;
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(cd->tv);
    GtkTreeModel *model = gtk_tree_view_get_model(priv->tv);
    GtkTreeIter iter;

    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, cd->index, new_text,
                       -1);
}

static void trg_persistent_tree_view_refresh(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(wd->widget);
    GtkListStore *model =
        GTK_LIST_STORE(gtk_tree_view_get_model(priv->tv));
    GtkTreeIter iter;
    JsonArray *ja;
    GList *ja_list, *li;
    GSList *sli;

    ja = trg_prefs_get_array(prefs, wd->key, wd->flags);

    gtk_list_store_clear(model);

    if (!ja)
        return;

    ja_list = json_array_get_elements(ja);

    for (li = ja_list; li; li = g_list_next(li)) {
        JsonNode *ja_node = (JsonNode *) li->data;
        JsonObject *jobj = json_node_get_object(ja_node);
        gtk_list_store_append(model, &iter);
        for (sli = priv->columns; sli; sli = g_slist_next(sli)) {
            trg_persistent_tree_view_column *cd =
                (trg_persistent_tree_view_column *) sli->data;
            gtk_list_store_set(model, &iter, cd->index,
                               json_object_get_string_member(jobj,
                                                             cd->key), -1);
        }
    }

    g_list_free(ja_list);
}

static gboolean
trg_persistent_tree_view_save_foreachfunc(GtkTreeModel *
                                          model,
                                          GtkTreePath *
                                          path,
                                          GtkTreeIter * iter,
                                          gpointer data)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(data);
    JsonObject *new = json_object_new();
    gchar *value;
    GSList *li;

    for (li = priv->columns; li; li = g_slist_next(li)) {
        trg_persistent_tree_view_column *cd =
            (trg_persistent_tree_view_column *) li->data;
        gtk_tree_model_get(model, iter, cd->index, &value, -1);
        json_object_set_string_member(new, cd->key, value);
        g_free(value);
    }

    json_array_add_object_element(priv->ja, new);

    return FALSE;
}

static void trg_persistent_tree_view_save(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(wd->widget);
    GtkTreeModel *model = gtk_tree_view_get_model(priv->tv);

    JsonNode *node = trg_prefs_get_value(prefs, wd->key, JSON_NODE_ARRAY,
                                         wd->flags |
                                         TRG_PREFS_REPLACENODE);

    priv->ja = json_array_new();

    gtk_tree_model_foreach(model,
                           trg_persistent_tree_view_save_foreachfunc,
                           wd->widget);
    json_node_take_array(node, priv->ja);

    trg_prefs_changed_emit_signal(prefs, wd->key);
}

trg_persistent_tree_view_column
    * trg_persistent_tree_view_add_column(TrgPersistentTreeView * ptv,
                                          gint index, const gchar * key,
                                          const gchar * label)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(ptv);
    trg_persistent_tree_view_column *cd =
        g_new0(trg_persistent_tree_view_column, 1);
    GtkCellRenderer *renderer;

    cd->key = g_strdup(key);
    cd->label = g_strdup(label);
    cd->index = index;
    cd->tv = ptv;

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited",
                     G_CALLBACK(trg_persistent_tree_view_edit), cd);
    cd->column = gtk_tree_view_column_new_with_attributes(cd->label,
                                                          renderer, "text",
                                                          cd->index, NULL);
    gtk_tree_view_column_set_resizable(cd->column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(priv->tv), cd->column);

    priv->columns = g_slist_append(priv->columns, cd);

    return cd;
}

static GtkTreeView
    * trg_persistent_tree_view_tree_view_new(TrgPersistentTreeView * ptv,
                                             GtkTreeModel * model)
{
    GtkTreeView *tv = GTK_TREE_VIEW(gtk_tree_view_new_with_model(model));
    GtkTreeSelection *selection;

    g_object_unref(G_OBJECT(model));

    gtk_tree_view_set_rubber_banding(tv, TRUE);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv));

    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(selection_changed_cb), ptv);

    return tv;
}

static void trg_persistent_tree_view_add_cb(GtkWidget * w, gpointer data)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(data);
    GtkTreeModel *model = gtk_tree_view_get_model(priv->tv);
    GtkTreeIter iter;
    GtkTreePath *path;

    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    path = gtk_tree_model_get_path(model, &iter);

    if (priv->addSelect)
        gtk_tree_view_set_cursor(priv->tv, path, priv->addSelect->column,
                                 TRUE);

    gtk_tree_path_free(path);
}

static void trg_persistent_tree_view_up_cb(GtkWidget * w, gpointer data)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(priv->tv);
    GtkTreeModel *model;
    GtkTreeIter iter, prevIter;
    GtkTreePath *path;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        path = gtk_tree_model_get_path(model, &iter);
        if (gtk_tree_path_prev(path) &&
            gtk_tree_model_get_iter(model, &prevIter, path)) {
            gtk_list_store_move_before(GTK_LIST_STORE(model), &iter,
                                       &prevIter);
            selection_changed(TRG_PERSISTENT_TREE_VIEW(data), selection);
        }
        gtk_tree_path_free(path);
    }
}

static void trg_persistent_tree_view_down_cb(GtkWidget * w, gpointer data)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(priv->tv);
    GtkTreeModel *model;
    GtkTreeIter iter, nextIter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        nextIter = iter;
        if (gtk_tree_model_iter_next(model, &nextIter)) {
            gtk_list_store_move_after(GTK_LIST_STORE(model), &iter,
                                      &nextIter);
            selection_changed(TRG_PERSISTENT_TREE_VIEW(data), selection);
        }
    }
}

static void trg_persistent_tree_view_del_cb(GtkWidget * w, gpointer data)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(priv->tv);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void
trg_persistent_tree_view_get_property(GObject * object,
                                      guint property_id,
                                      GValue * value, GParamSpec * pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_persistent_tree_view_set_property(GObject * object,
                                      guint property_id,
                                      const GValue * value,
                                      GParamSpec * pspec)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(object);
    switch (property_id) {
    case PROP_PREFS:
        priv->prefs = g_value_get_object(value);
        break;
    case PROP_KEY:
        priv->key = g_strdup(g_value_get_pointer(value));
        break;
    case PROP_MODEL:
        priv->model = g_value_get_object(value);
        break;
    case PROP_CONF_FLAGS:
    	priv->conf_flags = g_value_get_int(value);
    	break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_persistent_tree_view_finalize(GObject * object)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(object);
    GSList *li;
    for (li = priv->columns; li; li = g_slist_next(li)) {
        trg_persistent_tree_view_column *cd =
            (trg_persistent_tree_view_column *) li->data;
        g_free(cd->key);
        g_free(cd->label);
        g_free(cd);
    }
    g_slist_free(priv->columns);
    g_free(priv->key);
    G_OBJECT_CLASS(trg_persistent_tree_view_parent_class)->finalize
        (object);
}

trg_pref_widget_desc
    * trg_persistent_tree_view_get_widget_desc(TrgPersistentTreeView * ptv)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(ptv);
    return priv->wd;
}

void
trg_persistent_tree_view_set_add_select(TrgPersistentTreeView * ptv,
                                        trg_persistent_tree_view_column *
                                        cd)
{
    TrgPersistentTreeViewPrivate *priv = GET_PRIVATE(ptv);
    priv->addSelect = cd;
}

static GObject *trg_persistent_tree_view_constructor(GType type,
                                                     guint
                                                     n_construct_properties,
                                                     GObjectConstructParam
                                                     * construct_params)
{
    GObject *object;
    TrgPersistentTreeViewPrivate *priv;
    GtkWidget *hbox, *w;

    object = G_OBJECT_CLASS
        (trg_persistent_tree_view_parent_class)->constructor(type,
                                                             n_construct_properties,
                                                             construct_params);
    priv = GET_PRIVATE(object);

    hbox = trg_hbox_new(FALSE, 0);

    w = gtk_button_new_with_label(_("Add"));
    g_signal_connect(w, "clicked",
                     G_CALLBACK(trg_persistent_tree_view_add_cb), object);
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 4);

    w = priv->delButton = gtk_button_new_with_label(_("Delete"));
    gtk_widget_set_sensitive(w, FALSE);
    g_signal_connect(w, "clicked",
                     G_CALLBACK(trg_persistent_tree_view_del_cb), object);
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 4);

    w = priv->upButton = gtk_button_new_with_label(_("Up"));
    gtk_widget_set_sensitive(w, FALSE);
    g_signal_connect(w, "clicked",
                     G_CALLBACK(trg_persistent_tree_view_up_cb), object);
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 4);

    w = priv->downButton = gtk_button_new_with_label(_("Down"));
    gtk_widget_set_sensitive(w, FALSE);
    g_signal_connect(w, "clicked",
                     G_CALLBACK(trg_persistent_tree_view_down_cb), object);
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 4);

    priv->tv =
        trg_persistent_tree_view_tree_view_new(TRG_PERSISTENT_TREE_VIEW
                                               (object), priv->model);
    gtk_box_pack_start(GTK_BOX(object),
                       my_scrolledwin_new(GTK_WIDGET(priv->tv)), TRUE,
                       TRUE, 4);
    gtk_box_pack_start(GTK_BOX(object), hbox, FALSE, FALSE, 4);

    priv->wd = trg_pref_widget_desc_new(GTK_WIDGET(priv->tv), priv->key,
                                        priv->conf_flags);
    priv->wd->widget = GTK_WIDGET(object);
    priv->wd->saveFunc = &trg_persistent_tree_view_save;
    priv->wd->refreshFunc = &trg_persistent_tree_view_refresh;

    return object;
}

static void
trg_persistent_tree_view_class_init(TrgPersistentTreeViewClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgPersistentTreeViewPrivate));

    object_class->get_property = trg_persistent_tree_view_get_property;
    object_class->set_property = trg_persistent_tree_view_set_property;
    object_class->finalize = trg_persistent_tree_view_finalize;
    object_class->constructor = trg_persistent_tree_view_constructor;

    g_object_class_install_property(object_class,
                                    PROP_KEY,
                                    g_param_spec_pointer("conf-key",
                                                         "Conf Key",
                                                         "Conf Key",
                                                         G_PARAM_READWRITE
                                                         |
                                                         G_PARAM_CONSTRUCT_ONLY
                                                         |
                                                         G_PARAM_STATIC_NAME
                                                         |
                                                         G_PARAM_STATIC_NICK
                                                         |
                                                         G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_PREFS,
                                    g_param_spec_object("prefs",
                                                        "Prefs",
                                                        "Prefs",
                                                        TRG_TYPE_PREFS,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_CONF_FLAGS,
                                    g_param_spec_int("conf-flags",
                                                     "Conf Flags",
                                                     "Conf Flags",
                                                     INT_MIN,
                                                     INT_MAX,
                                                     TRG_PREFS_PROFILE,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY |
                                                     G_PARAM_STATIC_NAME |
                                                     G_PARAM_STATIC_NICK |
                                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_MODEL,
                                    g_param_spec_object("persistent-model",
                                                        "Persistent Model",
                                                        "Persistent Model",
                                                        GTK_TYPE_LIST_STORE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));
}

static void trg_persistent_tree_view_init(TrgPersistentTreeView * self)
{
}

TrgPersistentTreeView *trg_persistent_tree_view_new(TrgPrefs * prefs,
                                                    GtkListStore * model,
                                                    const gchar * key, gint conf_flags)
{
    GObject *obj =
        g_object_new(TRG_TYPE_PERSISTENT_TREE_VIEW, "prefs", prefs,
                     "conf-key", key, "persistent-model",
                     model, "conf-flags", conf_flags,
                     "orientation", GTK_ORIENTATION_VERTICAL,
                     NULL);

    return TRG_PERSISTENT_TREE_VIEW(obj);
}
