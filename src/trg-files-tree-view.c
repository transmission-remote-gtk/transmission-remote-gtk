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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-client.h"
#include "trg-tree-view.h"
#include "trg-files-model-common.h"
#include "trg-files-tree-view-common.h"
#include "trg-files-tree-view.h"
#include "trg-files-model.h"
#include "trg-main-window.h"
#include "requests.h"
#include "util.h"
#include "json.h"
#include "protocol-constants.h"

G_DEFINE_TYPE(TrgFilesTreeView, trg_files_tree_view, TRG_TYPE_TREE_VIEW)
#define TRG_FILES_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_FILES_TREE_VIEW, TrgFilesTreeViewPrivate))
typedef struct _TrgFilesTreeViewPrivate TrgFilesTreeViewPrivate;

struct _TrgFilesTreeViewPrivate {
    TrgClient *client;
    TrgMainWindow *win;
};

static void trg_files_tree_view_class_init(TrgFilesTreeViewClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgFilesTreeViewPrivate));
}

static gboolean
send_updated_file_prefs_foreachfunc(GtkTreeModel * model,
                                    GtkTreePath *
                                    path G_GNUC_UNUSED,
                                    GtkTreeIter * iter, gpointer data)
{
    JsonObject *args = (JsonObject *) data;
    gint priority;
    gint id;
    gint wanted;

    gtk_tree_model_get(model, iter, FILESCOL_ID, &id, -1);

    if (id < 0)
        return FALSE;

    gtk_tree_model_get(model, iter, FILESCOL_WANTED, &wanted,
                       FILESCOL_PRIORITY, &priority, -1);

    if (wanted)
        add_file_id_to_array(args, FIELD_FILES_WANTED, id);
    else
        add_file_id_to_array(args, FIELD_FILES_UNWANTED, id);

    if (priority == TR_PRI_LOW)
        add_file_id_to_array(args, FIELD_FILES_PRIORITY_LOW, id);
    else if (priority == TR_PRI_HIGH)
        add_file_id_to_array(args, FIELD_FILES_PRIORITY_HIGH, id);
    else
        add_file_id_to_array(args, FIELD_FILES_PRIORITY_NORMAL, id);

    return FALSE;
}

static gboolean on_files_update(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgFilesTreeViewPrivate *priv =
        TRG_FILES_TREE_VIEW_GET_PRIVATE(response->cb_data);
    GtkTreeModel *model =
        gtk_tree_view_get_model(GTK_TREE_VIEW(response->cb_data));

    trg_files_model_set_accept(TRG_FILES_MODEL(model), TRUE);

    response->cb_data = priv->win;

    return on_generic_interactive_action_response(data);
}

static void send_updated_file_prefs(TrgFilesTreeView * tv)
{
    TrgFilesTreeViewPrivate *priv = TRG_FILES_TREE_VIEW_GET_PRIVATE(tv);
    JsonNode *req;
    JsonObject *args;
    GtkTreeModel *model;
    gint64 targetId;
    JsonArray *targetIdArray;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    targetId = trg_files_model_get_torrent_id(TRG_FILES_MODEL(model));
    targetIdArray = json_array_new();
    json_array_add_int_element(targetIdArray, targetId);

    req = torrent_set(targetIdArray);
    args = node_get_arguments(req);

    gtk_tree_model_foreach(model, send_updated_file_prefs_foreachfunc,
                           args);

    trg_files_model_set_accept(TRG_FILES_MODEL(model), FALSE);

    dispatch_async(priv->client, req, on_files_update, tv);
}

static void set_low(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    trg_files_tree_model_set_priority(GTK_TREE_VIEW(data),
                                      FILESCOL_PRIORITY, TR_PRI_LOW);
    send_updated_file_prefs(TRG_FILES_TREE_VIEW(data));
}

static void set_normal(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    trg_files_tree_model_set_priority(GTK_TREE_VIEW(data),
                                      FILESCOL_PRIORITY, TR_PRI_NORMAL);
    send_updated_file_prefs(TRG_FILES_TREE_VIEW(data));
}

static void set_high(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    trg_files_tree_model_set_priority(GTK_TREE_VIEW(data),
                                      FILESCOL_PRIORITY, TR_PRI_HIGH);
    send_updated_file_prefs(TRG_FILES_TREE_VIEW(data));
}

static void set_unwanted(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    trg_files_model_set_wanted(GTK_TREE_VIEW(data), FILESCOL_WANTED,
                               FALSE);
    send_updated_file_prefs(TRG_FILES_TREE_VIEW(data));
}

static void set_wanted(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    trg_files_model_set_wanted(GTK_TREE_VIEW(data), FILESCOL_WANTED, TRUE);
    send_updated_file_prefs(TRG_FILES_TREE_VIEW(data));
}

static gboolean
view_onButtonPressed(GtkWidget * treeview,
                     GdkEventButton * event, gpointer userdata)
{
    gboolean handled =
        trg_files_tree_view_onViewButtonPressed(treeview, event,
                                                -1,
                                                FILESCOL_WANTED,
                                                G_CALLBACK(set_low),
                                                G_CALLBACK(set_normal),
                                                G_CALLBACK(set_high),
                                                G_CALLBACK(set_wanted),
                                                G_CALLBACK(set_unwanted),
                                                userdata);

    if (handled)
        send_updated_file_prefs(TRG_FILES_TREE_VIEW(treeview));

    return handled;
}

static gboolean
search_func (GtkTreeModel *model, gint column,
                 const gchar *key, GtkTreeIter *iter,
                 gpointer search_data)
{
    gchar *iter_string = NULL;
    gchar *lowercase;
    gboolean result = TRUE;
    gtk_tree_model_get(model, iter, column, &iter_string, -1);
    if (iter_string != NULL) {
        lowercase = g_utf8_strdown(iter_string, -1);
        result = g_strrstr(lowercase, key) == NULL;
    }
    g_free(lowercase);
    g_free(iter_string);
    return result;
}

static void trg_files_tree_view_init(TrgFilesTreeView * self)
{
    TrgTreeView *ttv = TRG_TREE_VIEW(self);
    trg_column_description *desc;

    desc = trg_tree_view_reg_column(ttv, TRG_COLTYPE_FILEICONTEXT,
                                    FILESCOL_NAME, _("Name"), "name", 0);
    desc->model_column_extra = FILESCOL_ID;

    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SIZE, FILESCOL_SIZE,
                             _("Size"), "size", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_PROG, FILESCOL_PROGRESS,
                             _("Progress"), "progress", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_WANTED, FILESCOL_WANTED,
                             _("Download"), "wanted", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_PRIO, FILESCOL_PRIORITY,
                             _("Priority"), "priority", 0);

    gtk_tree_view_set_search_column(GTK_TREE_VIEW(self), FILESCOL_NAME);

    gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(self),
                                        (GtkTreeViewSearchEqualFunc) search_func,
                                        gtk_tree_view_get_model(GTK_TREE_VIEW(self)),
                                        NULL);

    g_signal_connect(self, "button-press-event",
                     G_CALLBACK(view_onButtonPressed), NULL);
    g_signal_connect(self, "popup-menu",
                     G_CALLBACK(trg_files_tree_view_viewOnPopupMenu),
                     NULL);
}

TrgFilesTreeView *trg_files_tree_view_new(TrgFilesModel * model,
                                          TrgMainWindow * win,
                                          TrgClient * client,
                                          const gchar * configId)
{
    GObject *obj = g_object_new(TRG_TYPE_FILES_TREE_VIEW,
                                "config-id", configId,
                                "prefs", trg_client_get_prefs(client),
                                NULL);

    TrgFilesTreeViewPrivate *priv = TRG_FILES_TREE_VIEW_GET_PRIVATE(obj);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));

    priv->client = client;
    priv->win = win;

    trg_tree_view_setup_columns(TRG_TREE_VIEW(obj));

    return TRG_FILES_TREE_VIEW(obj);
}
