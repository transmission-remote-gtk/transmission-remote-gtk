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
#include <json-glib/json-glib.h>

#include "trg-files-model.h"
#include "trg-client.h"
#include "torrent.h"
#include "tfile.h"
#include "util.h"

#include "trg-files-model.h"

G_DEFINE_TYPE(TrgFilesModel, trg_files_model, GTK_TYPE_LIST_STORE)
#define TRG_FILES_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_FILES_MODEL, TrgFilesModelPrivate))
typedef struct _TrgFilesModelPrivate TrgFilesModelPrivate;

struct _TrgFilesModelPrivate {
    gint64 torrentId;
    JsonArray *wanted;
    JsonArray *priorities;
    gboolean accept;
};

static void trg_files_model_iter_new(TrgFilesModel * model,
                                     GtkTreeIter * iter, JsonObject * file,
                                     int id)
{
    gtk_list_store_append(GTK_LIST_STORE(model), iter);

    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       FILESCOL_NAME, file_get_name(file),
                       FILESCOL_SIZE, file_get_length(file),
                       FILESCOL_ID, id, -1);
}

void trg_files_model_set_accept(TrgFilesModel * model, gboolean accept)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    priv->accept = accept;
}

static void
trg_files_model_iter_update(TrgFilesModel * model,
                            GtkTreeIter * filesIter, JsonObject * file,
                            JsonArray * wantedArray,
                            JsonArray * prioritiesArray, int id)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);

    gboolean wanted = json_node_get_int(json_array_get_element
                                        (wantedArray, id)) == 1;
    gint64 priority =
        json_node_get_int(json_array_get_element(prioritiesArray, id));
    gdouble progress = file_get_progress(file);

    gtk_list_store_set(GTK_LIST_STORE(model), filesIter,
                       FILESCOL_PROGRESS, progress, -1);

    if (priv->accept) {
        gtk_list_store_set(GTK_LIST_STORE(model), filesIter,
                           FILESCOL_ICON,
                           wanted ? GTK_STOCK_FILE :
                           GTK_STOCK_STOP, FILESCOL_WANTED, wanted,
                           FILESCOL_PRIORITY, priority, -1);
    }
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

    column_types[FILESCOL_ICON] = G_TYPE_STRING;
    column_types[FILESCOL_NAME] = G_TYPE_STRING;
    column_types[FILESCOL_SIZE] = G_TYPE_INT64;
    column_types[FILESCOL_PROGRESS] = G_TYPE_DOUBLE;
    column_types[FILESCOL_ID] = G_TYPE_INT;
    column_types[FILESCOL_WANTED] = G_TYPE_BOOLEAN;
    column_types[FILESCOL_PRIORITY] = G_TYPE_INT64;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self),
                                    FILESCOL_COLUMNS, column_types);
}

gboolean
trg_files_model_update_foreach(GtkListStore * model,
                               GtkTreePath * path G_GNUC_UNUSED,
                               GtkTreeIter * iter,
                               GList *files)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    JsonObject *file;
    gint id;

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter, FILESCOL_ID, &id, -1);

    file = json_node_get_object(g_list_nth_data(files, id));
    trg_files_model_iter_update(TRG_FILES_MODEL(model), iter, file,
                                priv->wanted, priv->priorities, id);

    return FALSE;
}

void
trg_files_model_update(TrgFilesModel * model, gint64 updateSerial,
                       JsonObject * t, gint mode)
{
    TrgFilesModelPrivate *priv = TRG_FILES_MODEL_GET_PRIVATE(model);
    GList *filesList, *li;
    gint j = 0;

    priv->torrentId = torrent_get_id(t);
    priv->priorities = torrent_get_priorities(t);
    priv->wanted = torrent_get_wanted(t);

    filesList = json_array_get_elements(torrent_get_files(t));

    if (mode == TORRENT_GET_MODE_FIRST) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
        priv->accept = TRUE;
        for (li = filesList; li; li = g_list_next(li)) {
            JsonObject *file = json_node_get_object((JsonNode *) li->data);
            GtkTreeIter filesIter;
            trg_files_model_iter_new(model, &filesIter, file, j);
            trg_files_model_iter_update(model, &filesIter,
                                        file, priv->wanted,
                                        priv->priorities, j);
            j++;
        }
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
