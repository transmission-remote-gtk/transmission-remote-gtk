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

static void iter_to_row_reference(GtkTreeModel *model, GtkTreeIter *iter,
        GtkTreeRowReference **rr) {
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);

    if (*rr)
        gtk_tree_row_reference_free(*rr);

    *rr = gtk_tree_row_reference_new(model, path);
    gtk_tree_path_free(path);
}

static void rowref_to_iter(GtkTreeModel *model, GtkTreeRowReference *rr,
        GtkTreeIter *iter) {
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    gtk_tree_model_get_iter(model, iter, path);
    gtk_tree_path_free(path);
}

static gboolean trg_files_update_new_parents(GtkTreeModel *model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
    GtkTreeIter *descendentIter = (GtkTreeIter*) data;
    GtkTreePath *descendentPath = gtk_tree_model_get_path(model,
            descendentIter);

    if (gtk_tree_path_is_ancestor(path, descendentPath)
            && gtk_tree_model_get_iter(model, descendentIter, descendentPath)) {
        gint64 size, oldSize;
        gtk_tree_model_get(model, descendentIter, FILESCOL_SIZE, &size, -1);
        gtk_tree_model_get(model, iter, FILESCOL_SIZE, &oldSize, -1);
        gtk_tree_store_set(GTK_TREE_STORE(model), iter, FILESCOL_SIZE,
                size + oldSize, -1);
    }

    gtk_tree_path_free(descendentPath);

    return FALSE;
}

struct updateAllArgs {
    GtkTreeIter *descendentIter;
    gint64 increment;
};

static gboolean trg_files_update_all_parents(GtkTreeModel *model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
    struct updateAllArgs *args = (struct updateAllArgs*) data;

    GtkTreePath *descendentPath = gtk_tree_model_get_path(model,
            args->descendentIter);

    if (gtk_tree_path_is_ancestor(path, descendentPath)) {
        gint64 lastCompleted, newCompleted, length;

        gtk_tree_model_get(model, iter, FILESCOL_BYTESCOMPLETED, &lastCompleted,
                FILESCOL_SIZE, &length, -1);
        newCompleted = lastCompleted + args->increment;

        gtk_tree_store_set(GTK_TREE_STORE(model), iter, FILESCOL_BYTESCOMPLETED,
                newCompleted, FILESCOL_PROGRESS,
                file_get_progress(length, newCompleted), -1);

    }

    gtk_tree_path_free(descendentPath);

    return FALSE;
}

static void trg_files_model_iter_new(TrgFilesModel * model, GtkTreeIter * iter,
        JsonObject * file, gint id) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    gchar **elements = g_strsplit(file_get_name(file), "/", -1);
    gchar *existingName;
    gint i, existingId;
    GtkTreeRowReference *parentRowRef = NULL;
    GtkTreeIter parentIter;

    for (i = 0; elements[i]; i++) {
        GtkTreeIter *found = NULL;

        if (parentRowRef)
            rowref_to_iter(GTK_TREE_MODEL(model), parentRowRef, &parentIter);

        /* If this is the last component of the path, create a file node. */

        if (!elements[i + 1]) {
            gtk_tree_store_append(GTK_TREE_STORE(model), iter,
                    parentRowRef ? &parentIter : NULL);
            gtk_tree_store_set(GTK_TREE_STORE(model), iter, FILESCOL_NAME,
                    elements[i], FILESCOL_SIZE, file_get_length(file),
                    FILESCOL_ID, id, -1);

            if (parentRowRef) {
                gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                        trg_files_update_new_parents, iter);
            }

            break;
        }

        /* Search for the directory this files goes under, under the saved
         * GtkTreeRowReferece *parent. */

        if (gtk_tree_model_iter_children(GTK_TREE_MODEL(model), iter,
                parentRowRef ? &parentIter : NULL)) {
            do {
                gtk_tree_model_get(GTK_TREE_MODEL(model), iter, FILESCOL_NAME,
                        &existingName, FILESCOL_ID, &existingId, -1);

                if (existingId == -1 && !g_strcmp0(elements[i], existingName)) {
                    found = iter;
                    iter_to_row_reference(GTK_TREE_MODEL(model), iter,
                            &parentRowRef);
                }

                g_free(existingName);

                if (found)
                    break;
            } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(model), iter));
        }

        if (!found) {
            GValue gvalue = { 0 };

            gtk_tree_store_append(GTK_TREE_STORE(model), iter,
                    parentRowRef ? &parentIter : NULL);
            gtk_tree_store_set(GTK_TREE_STORE(model), iter, FILESCOL_NAME,
                    elements[i], -1);

            g_value_init(&gvalue, G_TYPE_INT);
            g_value_set_int(&gvalue, -1);
            gtk_tree_store_set_value(GTK_TREE_STORE(model), iter, FILESCOL_ID,
                    &gvalue);

            memset(&gvalue, 0, sizeof(GValue));
            g_value_init(&gvalue, G_TYPE_INT64);
            g_value_set_int64(&gvalue, TR_PRI_UNSET);
            gtk_tree_store_set_value(GTK_TREE_STORE(model), iter,
                    FILESCOL_PRIORITY, &gvalue);

            iter_to_row_reference(GTK_TREE_MODEL(model), iter, &parentRowRef);
        }
    }

    if (parentRowRef)
        gtk_tree_row_reference_free(parentRowRef);

    g_strfreev(elements);
    priv->n_items++;
}

void trg_files_model_set_accept(TrgFilesModel * model, gboolean accept) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    priv->accept = accept;
}

static void trg_files_model_iter_update(TrgFilesModel * model,
        GtkTreeIter * filesIter, gboolean isFirst, JsonObject * file,
        JsonArray * wantedArray, JsonArray * prioritiesArray, int id) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);

    gint64 fileLength = file_get_length(file);
    gint64 fileCompleted = file_get_bytes_completed(file);

    gboolean wanted = json_node_get_int(json_array_get_element(wantedArray, id))
            == 1;
    gint64 priority = json_node_get_int(
            json_array_get_element(prioritiesArray, id));
    gdouble progress = file_get_progress(fileLength, fileCompleted);

    struct updateAllArgs args;

    if (isFirst) {
        args.increment = fileCompleted;
    } else {
        gint64 lastCompleted;
        gtk_tree_model_get(GTK_TREE_MODEL(model), filesIter,
                FILESCOL_BYTESCOMPLETED, &lastCompleted, -1);
        args.increment = fileCompleted - lastCompleted;
    }

    gtk_tree_store_set(GTK_TREE_STORE(model), filesIter, FILESCOL_PROGRESS,
            progress, FILESCOL_BYTESCOMPLETED, fileCompleted, -1);

    if (args.increment > 0) {
        args.descendentIter = filesIter;
        gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                trg_files_update_all_parents, &args);
    }

    if (priv->accept) {
        gtk_tree_store_set(GTK_TREE_STORE(model), filesIter, FILESCOL_WANTED,
                wanted ? GTK_STOCK_APPLY : GTK_STOCK_CANCEL
                , FILESCOL_PRIORITY, priority, -1);

    }
}

static void trg_files_model_class_init(TrgFilesModelClass * klass) {
    g_type_class_add_private(klass, sizeof(TrgFilesModelPrivate));
}

static void trg_files_model_init(TrgFilesModel * self) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(self);
    GType column_types[FILESCOL_COLUMNS];

    priv->accept = TRUE;

    column_types[FILESCOL_NAME] = G_TYPE_STRING;
    column_types[FILESCOL_SIZE] = G_TYPE_INT64;
    column_types[FILESCOL_PROGRESS] = G_TYPE_DOUBLE;
    column_types[FILESCOL_ID] = G_TYPE_INT;
    column_types[FILESCOL_WANTED] = G_TYPE_STRING;
    column_types[FILESCOL_PRIORITY] = G_TYPE_INT64;
    column_types[FILESCOL_BYTESCOMPLETED] = G_TYPE_INT64;

    gtk_tree_store_set_column_types(GTK_TREE_STORE(self), FILESCOL_COLUMNS,
            column_types);
}

gboolean trg_files_model_update_foreach(GtkListStore * model,
        GtkTreePath * path G_GNUC_UNUSED, GtkTreeIter * iter, GList * files) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    JsonObject *file;
    gint id;

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter, FILESCOL_ID, &id, -1);

    if (id >= 0) {
        file = json_node_get_object(g_list_nth_data(files, id));
        trg_files_model_iter_update(TRG_FILES_MODEL(model), iter, FALSE, file,
                priv->wanted, priv->priorities, id);
    }

    return FALSE;
}

void trg_files_model_update(TrgFilesModel * model, gint64 updateSerial,
        JsonObject * t, gint mode) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    GList *filesList, *li;
    GtkTreeIter filesIter;
    JsonObject *file;
    gint j = 0;

    priv->torrentId = torrent_get_id(t);
    priv->priorities = torrent_get_priorities(t);
    priv->wanted = torrent_get_wanted(t);

    filesList = json_array_get_elements(torrent_get_files(t));

    if (mode == TORRENT_GET_MODE_FIRST) {
        gtk_tree_store_clear(GTK_TREE_STORE(model));
        priv->accept = TRUE;
        for (li = filesList; li; li = g_list_next(li)) {
            file = json_node_get_object((JsonNode *) li->data);

            trg_files_model_iter_new(model, &filesIter, file, j);
            trg_files_model_iter_update(model, &filesIter, TRUE, file,
                    priv->wanted, priv->priorities, j);
            j++;
        }
    } else {
        guint n_updates = g_list_length(filesList);
        gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                (GtkTreeModelForeachFunc) trg_files_model_update_foreach,
                filesList);
        if (n_updates > priv->n_items) {
            gint n_new = n_updates - priv->n_items;
            for (j = n_updates - n_new; j < n_updates; j++) {
                file = json_node_get_object(g_list_nth_data(filesList, j));
                trg_files_model_iter_new(model, &filesIter, file, j);
                trg_files_model_iter_update(model, &filesIter, TRUE, file,
                        priv->wanted, priv->priorities, j);
            }
        }
    }

    g_list_free(filesList);
}

gint64 trg_files_model_get_torrent_id(TrgFilesModel * model) {
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    return priv->torrentId;
}

TrgFilesModel *trg_files_model_new(void) {
    return g_object_new(TRG_TYPE_FILES_MODEL, NULL);
}
