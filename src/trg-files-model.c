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

#include <string.h>
#include <limits.h>
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
    gboolean accept;
};

/* Push a given increment to a treemodel node and its parents.
 * Used for updating bytescompleted.
 * This is only used for user interaction, the initial population is done
 * in a simple tree before for performance.
 * */

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

/* Update names for all nodes parents, i.e. folders
 */
static void trg_files_update_parent_names(GtkTreeModel *model,
                                          GtkTreeIter *iter,
                                          gchar **path_last)
{
    GtkTreeIter back_iter = *iter;

    while (1) {
        GtkTreeIter tmp_iter;

        if (!gtk_tree_model_iter_parent(model, &tmp_iter, &back_iter))
            break;

        --path_last;

        /* Is it better to test if name has changed before calling store_set? */
        gtk_tree_store_set(GTK_TREE_STORE(model), &tmp_iter,
                           FILESCOL_NAME, *path_last, -1);

        back_iter = tmp_iter;
    }
}

/* Update the bytesCompleted and size for a nodes parents, and also figure out
 * if a priority/enabled change requires updating the parents (needs to iterate
 * over the other nodes at its level).
 *
 * It's faster doing it in here than when it's in the model.
 */
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
            gint common_result = 0;

            if (back_node->priority != pri_result)
                common_result = pri_result = TR_PRI_MIXED;

            if (back_node->enabled != enabled_result)
                common_result = enabled_result = TR_PRI_MIXED;

            if (common_result == TR_PRI_MIXED)
                break;
        }

        back_iter->bytesCompleted += node->bytesCompleted;
        back_iter->length += node->length;
        back_iter->priority = pri_result;
        back_iter->enabled = enabled_result;
    }
}

static void
store_add_node(GtkTreeStore * store, GtkTreeIter * parent,
               trg_files_tree_node * node)
{
    GtkTreeIter child;
    GList *li;

    if (node->name) {
        gdouble progress =
            file_get_progress(node->length, node->bytesCompleted);
        gtk_tree_store_insert_with_values(store, &child, parent, INT_MAX,
                                          FILESCOL_WANTED, node->enabled,
                                          FILESCOL_PROGRESS, progress,
                                          FILESCOL_SIZE, node->length,
                                          FILESCOL_ID, node->index,
                                          FILESCOL_PRIORITY,
                                          node->priority, FILESCOL_NAME,
                                          node->name, -1);
    }

    for (li = node->children; li; li = g_list_next(li))
        store_add_node(store, node->name ? &child : NULL,
                       (trg_files_tree_node *) li->data);
}

static trg_files_tree_node *trg_file_parser_node_insert(trg_files_tree_node
                                                        * top,
                                                        trg_files_tree_node
                                                        * last,
                                                        JsonObject * file,
                                                        gint index,
                                                        JsonArray *
                                                        enabled,
                                                        JsonArray *
                                                        priorities)
{
    gchar **path = g_strsplit(file_get_name(file), "/", -1);
    trg_files_tree_node *lastIter = last;
    GList *parentList = NULL;
    gchar *path_el;
    GList *li;
    int i;

    /* Build up a list of pointers to each parent trg_files_tree_node
     * reversing the order as it iterates over its parent.
     */
    if (lastIter)
        while ((lastIter = lastIter->parent))
            parentList = g_list_prepend(parentList, lastIter);

    li = parentList;
    lastIter = top;

    /* Iterate over the path list which contains each file/directory
     * component of the path in order.
     */
    for (i = 0; (path_el = path[i]); i++) {
        gboolean isFile = !path[i + 1];
        trg_files_tree_node *target_node = NULL;

        /* No point checking for files. If there is a last parents iterator
         * check it for a shortcut. I'm assuming that these come in order of
         * directory at least to give performance a boost.
         */
        if (li && !isFile) {
            trg_files_tree_node *lastPathNode =
                (trg_files_tree_node *) li->data;

            if (!g_strcmp0(lastPathNode->name, path[i])) {
                target_node = lastPathNode;
                li = g_list_next(li);
            } else {
                /* No need to check any further. */
                li = NULL;
            }
        }

        if (!target_node && lastIter && lastIter->childrenHash && !isFile)
          target_node = g_hash_table_lookup(lastIter->childrenHash, path_el);

        /* Node needs creating */

        if (!target_node) {
            target_node = g_new0(trg_files_tree_node, 1);
            target_node->name = g_strdup(path[i]);
            target_node->parent = lastIter;
            trg_files_tree_node_add_child(lastIter, target_node);
        }

        lastIter = target_node;

        /* Files have more properties set here than for files.
         * Directories are updated from here too, by trg_files_tree_update_ancestors
         * working up the parents.
         */
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
    g_strfreev(path);

    return lastIter;
}

void trg_files_model_set_accept(TrgFilesModel * model, gboolean accept)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    priv->accept = accept;
}

static void
trg_files_model_iter_update(TrgFilesModel * model,
                            GtkTreeIter * filesIter,
                            JsonObject * file,
                            JsonArray * wantedArray,
                            JsonArray * prioritiesArray, gint id)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    gchar **path = g_strsplit(file_get_name(file), "/", -1);

    /* Find last element in the path */
    gchar **path_last;
    for (path_last = path; path_last[1]; ++path_last);

    gint64 fileLength = file_get_length(file);
    gint64 fileCompleted = file_get_bytes_completed(file);
    gint64 lastCompleted;

    gint wanted = (gint) json_array_get_int_element(wantedArray, id);
    gint priority = (gint) json_array_get_int_element(prioritiesArray, id);
    gdouble progress = file_get_progress(fileLength, fileCompleted);

    gtk_tree_model_get(GTK_TREE_MODEL(model), filesIter,
                       FILESCOL_BYTESCOMPLETED, &lastCompleted, -1);

    gtk_tree_store_set(GTK_TREE_STORE(model), filesIter, FILESCOL_PROGRESS,
                       progress, FILESCOL_BYTESCOMPLETED, fileCompleted,
                       -1);

    trg_files_update_parent_progress(GTK_TREE_MODEL(model), filesIter,
                                     fileCompleted - lastCompleted);

    if (priv->accept) {
        gtk_tree_store_set(GTK_TREE_STORE(model), filesIter,
                           FILESCOL_NAME, *path_last,
                           FILESCOL_WANTED, wanted, FILESCOL_PRIORITY,
                           priority, -1);

        trg_files_update_parent_names(GTK_TREE_MODEL(model), filesIter,
                                      path_last);
    }

    g_strfreev(path);
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

struct MinorUpdateData {
    GList *filesList;
    JsonArray *priorities;
    JsonArray *wanted;
};

static gboolean
trg_files_model_update_foreach(GtkListStore * model,
                               GtkTreePath * path G_GNUC_UNUSED,
                               GtkTreeIter * iter, gpointer data)
{
    struct MinorUpdateData *mud = (struct MinorUpdateData *) data;
    JsonObject *file;
    gint id;

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter, FILESCOL_ID, &id, -1);

    if (id >= 0) {
        file = json_node_get_object(g_list_nth_data(mud->filesList, id));
        trg_files_model_iter_update(TRG_FILES_MODEL(model), iter,
                                    file, mud->wanted, mud->priorities,
                                    id);
    }

    return FALSE;
}

struct FirstUpdateThreadData {
    TrgFilesModel *model;
    GtkTreeView *tree_view;
    JsonArray *files;
    JsonArray *priorities;
    JsonArray *wanted;
    guint n_items;
    trg_files_tree_node *top_node;
    gint64 torrent_id;
    GList *filesList;
    gboolean idle_add;
};

static gboolean trg_files_model_applytree_idlefunc(gpointer data)
{
    struct FirstUpdateThreadData *args =
        (struct FirstUpdateThreadData *) data;
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(args->model);

    if (args->torrent_id == priv->torrentId) {
        store_add_node(GTK_TREE_STORE(args->model), NULL, args->top_node);
        gtk_tree_view_expand_all(args->tree_view);
        priv->n_items = args->n_items;
        priv->accept = TRUE;
    }

    trg_files_tree_node_free(args->top_node);
    g_free(data);

    return FALSE;
}

static gpointer trg_files_model_buildtree_threadfunc(gpointer data)
{
    struct FirstUpdateThreadData *args =
        (struct FirstUpdateThreadData *) data;
    trg_files_tree_node *lastNode = NULL;
    GList *li;

    args->top_node = g_new0(trg_files_tree_node, 1);

    for (li = args->filesList; li; li = g_list_next(li)) {
        JsonObject *file = json_node_get_object((JsonNode *) li->data);

        lastNode =
            trg_file_parser_node_insert(args->top_node, lastNode,
                                        file, args->n_items++,
                                        args->wanted, args->priorities);
    }

    g_list_free(args->filesList);
    json_array_unref(args->files);

    if (args->idle_add)
        g_idle_add(trg_files_model_applytree_idlefunc, data);

    return NULL;
}

void
trg_files_model_update(TrgFilesModel * model, GtkTreeView * tv,
                       gint64 updateSerial, JsonObject * t, gint mode)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    JsonArray *files = torrent_get_files(t);
    GList *filesList = json_array_get_elements(files);
    guint filesListLength = g_list_length(filesList);
    JsonArray *priorities = torrent_get_priorities(t);
    JsonArray *wanted = torrent_get_wanted(t);
    priv->torrentId = torrent_get_id(t);

    /* It's quicker to build this up with simple data structures before
     * putting it into GTK models.
     */
    if (mode == TORRENT_GET_MODE_FIRST || priv->n_items != filesListLength) {
        struct FirstUpdateThreadData *futd =
            g_new0(struct FirstUpdateThreadData, 1);

        gtk_tree_store_clear(GTK_TREE_STORE(model));
        json_array_ref(files);

        futd->tree_view = tv;
        futd->files = files;
        futd->priorities = priorities;
        futd->wanted = wanted;
        futd->filesList = filesList;
        futd->torrent_id = priv->torrentId;
        futd->model = model;
        futd->idle_add =
            filesListLength > TRG_FILES_MODEL_CREATE_THREAD_IF_GT;

        /* If this update has more than a given number of files, build up the
         * simple tree in a thread, then g_idle_add a function which
         * adds the contents of this prebuilt tree.
         *
         * If less than or equal to, I don't think it's worth spawning threads
         * for. Just do it in the main loop.
         */
        if (futd->idle_add) {
            g_thread_create(trg_files_model_buildtree_threadfunc, futd,
                            FALSE, NULL);
        } else {
            trg_files_model_buildtree_threadfunc(futd);
            trg_files_model_applytree_idlefunc(futd);
        }
    } else {
        struct MinorUpdateData mud;
        mud.priorities = priorities;
        mud.wanted = wanted;
        mud.filesList = filesList;
        gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                               (GtkTreeModelForeachFunc)
                               trg_files_model_update_foreach, &mud);
        g_list_free(filesList);
    }
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
