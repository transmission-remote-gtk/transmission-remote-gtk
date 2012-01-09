/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
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

#include <string.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "protocol-constants.h"
#include "trg-files-model-common.h"
#include "trg-files-tree-view-common.h"
#include "trg-files-tree.h"
#include "trg-files-model.h"
#include "trg-client.h"
#include "torrent.h"
#include "util.h"

#include "trg-files-model.h"

G_DEFINE_TYPE(TrgFilesModel, trg_files_model, GTK_TYPE_TREE_STORE)
#define TRG_FILES_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_FILES_MODEL, TrgFilesModelPrivate))
typedef struct _TrgFilesModelPrivate TrgFilesModelPrivate;

struct _TrgFilesModelPrivate {
    gint64 torrentId;
    guint n_items;
    JsonArray *wanted;
    JsonArray *priorities;
    gboolean accept;
};

static void trg_files_update_parent_progress(GtkTreeModel * model,
                                             GtkTreeIter * iter,
                                             gint64 increment)
{
    GtkTreeIter back_iter = *iter;

    if (increment < 1)
        return;

    while (1) {
        GtkTreeIter tmp_iter;
        gint64 lastCompleted, newCompleted, length;

        if (!gtk_tree_model_iter_parent(model, &tmp_iter, &back_iter))
            break;

        gtk_tree_model_get(model, &tmp_iter, FILESCOL_BYTESCOMPLETED,
                           &lastCompleted, FILESCOL_SIZE, &length, -1);
        newCompleted = lastCompleted + increment;

        gtk_tree_store_set(GTK_TREE_STORE(model), &tmp_iter,
                           FILESCOL_PROGRESS, file_get_progress(length,
                                                                newCompleted),
                           FILESCOL_BYTESCOMPLETED, newCompleted, -1);

        back_iter = tmp_iter;
    }
}

static void trg_files_tree_update_ancestors(trg_files_tree_node * node)
{
    trg_files_tree_node *back_iter = node;
    gint pri_result = node->priority;
    gint enabled_result = node->enabled;

    while ((back_iter = back_iter->parent)) {
        GList *li;
        for (li = back_iter->children; li; li = g_list_next(li)) {
            trg_files_tree_node *back_node =
                (trg_files_tree_node *) li->data;
            gboolean stop = FALSE;

            if (back_node->priority != pri_result) {
                pri_result = TR_PRI_MIXED;
                stop = TRUE;
            }

            if (back_node->enabled != enabled_result) {
                enabled_result = TR_PRI_MIXED;
                stop = TRUE;
            }

            if (stop)
                break;
        }

        back_iter->bytesCompleted += node->bytesCompleted;
        back_iter->length += node->length;
        back_iter->priority = pri_result;
        back_iter->enabled = enabled_result;
    }
}

static void store_add_node(GtkTreeStore * store, GtkTreeIter * parent,
                           trg_files_tree_node * node)
{
    GtkTreeIter child;
    GList *li;

    if (node->name) {
        gdouble progress =
            file_get_progress(node->length, node->bytesCompleted);
        gtk_tree_store_append(store, &child, parent);
        gtk_tree_store_set(store, &child, FILESCOL_WANTED, node->enabled,
                           FILESCOL_PROGRESS, progress,
                           FILESCOL_SIZE, node->length,
                           FILESCOL_ID, node->index,
                           FILESCOL_PRIORITY, node->priority,
                           FILESCOL_NAME, node->name, -1);
    }

    for (li = node->children; li; li = g_list_next(li))
        store_add_node(store, node->name ? &child : NULL,
                       (trg_files_tree_node *) li->data);
}

static trg_files_tree_node
    * trg_file_parser_node_insert(trg_files_tree_node * top,
                                  trg_files_tree_node * last,
                                  JsonObject * file, gint index,
                                  JsonArray * enabled,
                                  JsonArray * priorities)
{
    gchar **path = g_strsplit(file_get_name(file), "/", -1);
    trg_files_tree_node *lastIter = last;
    GList *parentList = NULL;
    gchar *path_el;
    GList *li;
    int i;

    if (lastIter)
        while ((lastIter = lastIter->parent))
            parentList = g_list_prepend(parentList, lastIter);

    li = parentList;
    lastIter = NULL;

    /* Iterate over the path list which contains each file/directory
     * component of the path in order.
     */
    for (i = 0; (path_el = path[i]); i++) {
        gboolean isFile = !path[i + 1];
        trg_files_tree_node *target_node = NULL;

        if (li && !isFile) {
            trg_files_tree_node *lastPathNode =
                (trg_files_tree_node *) li->data;

            if (!g_strcmp0(lastPathNode->name, path[i]))
                target_node = lastPathNode;

            li = g_list_next(li);
        }

        if (!target_node) {
            target_node = g_new0(trg_files_tree_node, 1);
            target_node->name = g_strdup(path[i]);
            target_node->parent = lastIter;

            if (lastIter)
                lastIter->children =
                    g_list_append(lastIter->children, target_node);
            else
                top->children = g_list_append(top->children, target_node);
        }

        lastIter = target_node;

        if (isFile) {
            target_node->length = file_get_length(file);
            target_node->bytesCompleted = file_get_bytes_completed(file);
            target_node->index = index;
            target_node->enabled =
                (gint) json_array_get_int_element(enabled, index);
            target_node->priority =
                (gint) json_array_get_int_element(priorities, index);

            trg_files_tree_update_ancestors(target_node);
        } else {
            target_node->index = -1;
        }
    }

    g_list_free(parentList);

    return lastIter;
}

void trg_files_model_set_accept(TrgFilesModel * model, gboolean accept)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    priv->accept = accept;
}

static void trg_files_model_iter_update(TrgFilesModel * model,
                                        GtkTreeIter * filesIter,
                                        JsonObject * file,
                                        JsonArray * wantedArray,
                                        JsonArray * prioritiesArray,
                                        gint id)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    gint64 fileLength = file_get_length(file);
    gint64 fileCompleted = file_get_bytes_completed(file);
    gint64 lastCompleted;

    gboolean wanted =
        json_node_get_int(json_array_get_element(wantedArray, id))
        == 1;
    gint priority = (gint)
        json_node_get_int(json_array_get_element(prioritiesArray, id));
    gdouble progress = file_get_progress(fileLength, fileCompleted);

    gtk_tree_model_get(GTK_TREE_MODEL(model), filesIter,
                       FILESCOL_BYTESCOMPLETED, &lastCompleted, -1);

    gtk_tree_store_set(GTK_TREE_STORE(model), filesIter, FILESCOL_PROGRESS,
                       progress, FILESCOL_BYTESCOMPLETED, fileCompleted,
                       -1);

    trg_files_update_parent_progress(GTK_TREE_MODEL(model), filesIter,
                                     fileCompleted - lastCompleted);

    if (priv->accept)
        gtk_tree_store_set(GTK_TREE_STORE(model), filesIter,
                           FILESCOL_WANTED, wanted, FILESCOL_PRIORITY,
                           priority, -1);
}

static void trg_files_model_class_init(TrgFilesModelClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgFilesModelPrivate));
}

static void trg_files_model_init(TrgFilesModel * self)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(self);
    GType column_types[FILESCOL_COLUMNS];

    priv->accept = TRUE;

    column_types[FILESCOL_NAME] = G_TYPE_STRING;
    column_types[FILESCOL_SIZE] = G_TYPE_INT64;
    column_types[FILESCOL_PROGRESS] = G_TYPE_DOUBLE;
    column_types[FILESCOL_ID] = G_TYPE_INT;
    column_types[FILESCOL_WANTED] = G_TYPE_INT;
    column_types[FILESCOL_PRIORITY] = G_TYPE_INT;
    column_types[FILESCOL_BYTESCOMPLETED] = G_TYPE_INT64;

    gtk_tree_store_set_column_types(GTK_TREE_STORE(self), FILESCOL_COLUMNS,
                                    column_types);
}

gboolean trg_files_model_update_foreach(GtkListStore * model,
                                        GtkTreePath * path G_GNUC_UNUSED,
                                        GtkTreeIter * iter, GList * files)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    JsonObject *file;
    gint id;

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter, FILESCOL_ID, &id, -1);

    if (id >= 0) {
        file = json_node_get_object(g_list_nth_data(files, id));
        trg_files_model_iter_update(TRG_FILES_MODEL(model), iter,
                                    file, priv->wanted, priv->priorities,
                                    id);
    }

    return FALSE;
}

void trg_files_model_update(TrgFilesModel * model, gint64 updateSerial,
                            JsonObject * t, gint mode)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    GList *filesList, *li;
    JsonObject *file;
    gint j = 0;
    guint n_updates;

    priv->torrentId = torrent_get_id(t);
    priv->priorities = torrent_get_priorities(t);
    priv->wanted = torrent_get_wanted(t);

    filesList = json_array_get_elements(torrent_get_files(t));
    n_updates = g_list_length(filesList);

    if (mode == TORRENT_GET_MODE_FIRST || priv->n_items != n_updates) {
        trg_files_tree_node *top_node = g_new0(trg_files_tree_node, 1);
        trg_files_tree_node *lastNode = NULL;
        gtk_tree_store_clear(GTK_TREE_STORE(model));
        priv->accept = TRUE;

        for (li = filesList; li; li = g_list_next(li)) {
            file = json_node_get_object((JsonNode *) li->data);

            lastNode =
                trg_file_parser_node_insert(top_node, lastNode,
                                            file, j++, priv->wanted,
                                            priv->priorities);
        }

        priv->n_items = j;

        store_add_node(GTK_TREE_STORE(model), NULL, top_node);

    } else {
        gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                               (GtkTreeModelForeachFunc)
                               trg_files_model_update_foreach, filesList);
    }

    g_list_free(filesList);
}

gint64 trg_files_model_get_torrent_id(TrgFilesModel * model)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    return priv->torrentId;
}

TrgFilesModel *trg_files_model_new(void)
{
    return g_object_new(TRG_TYPE_FILES_MODEL, NULL);
}
