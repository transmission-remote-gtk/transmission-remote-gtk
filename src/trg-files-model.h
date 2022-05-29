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

#ifndef TRG_FILES_MODEL_H_
#define TRG_FILES_MODEL_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-model.h"

G_BEGIN_DECLS
#define TRG_TYPE_FILES_MODEL trg_files_model_get_type()
#define TRG_FILES_MODEL(obj)                                                                       \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_FILES_MODEL, TrgFilesModel))
#define TRG_FILES_MODEL_CLASS(klass)                                                               \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_FILES_MODEL, TrgFilesModelClass))
#define TRG_IS_FILES_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_FILES_MODEL))
#define TRG_IS_FILES_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_FILES_MODEL))
#define TRG_FILES_MODEL_GET_CLASS(obj)                                                             \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_FILES_MODEL, TrgFilesModelClass))
typedef struct {
    GtkTreeStore parent;
} TrgFilesModel;

typedef struct {
    GtkTreeStoreClass parent_class;
} TrgFilesModelClass;

GType trg_files_model_get_type(void);

TrgFilesModel *trg_files_model_new(void);

G_END_DECLS enum {
    FILESCOL_NAME,
    FILESCOL_SIZE,
    FILESCOL_PROGRESS,
    FILESCOL_ID,
    FILESCOL_WANTED,
    FILESCOL_PRIORITY,
    FILESCOL_BYTESCOMPLETED,
    FILESCOL_COLUMNS
};

#define TRG_FILES_MODEL_CREATE_THREAD_IF_GT 600

void trg_files_model_update(TrgFilesModel *model, GtkTreeView *tv, gint64 updateSerial,
                            JsonObject *t, gint mode);
gint64 trg_files_model_get_torrent_id(TrgFilesModel *model);
void trg_files_model_set_accept(TrgFilesModel *model, gboolean accept);

#endif /* TRG_FILES_MODEL_H_ */
