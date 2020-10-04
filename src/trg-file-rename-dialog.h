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

#ifndef TRG_FILE_RENAME_DIALOG_H_
#define TRG_FILE_RENAME_DIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-client.h"
#include "trg-main-window.h"
#include "trg-files-tree-view.h"

G_BEGIN_DECLS
#define TRG_TYPE_FILE_RENAME_DIALOG trg_file_rename_dialog_get_type()
#define TRG_FILE_RENAME_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_FILE_RENAME_DIALOG, TrgFileRenameDialog))
#define TRG_FILE_RENAME_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_FILE_RENAME_DIALOG, TrgFileRenameDialogClass))
#define TRG_IS_FILE_RENAME_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_FILE_RENAME_DIALOG))
#define TRG_IS_FILE_RENAME_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_FILE_RENAME_DIALOG))
#define TRG_FILE_RENAME_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_FILE_RENAME_DIALOG, TrgFileRenameDialogClass))

typedef struct {
    GtkDialog parent;
} TrgFileRenameDialog;

typedef struct {
    GtkDialogClass parent_class;
} TrgFileRenameDialogClass;

GType trg_file_rename_dialog_get_type(void);

TrgFileRenameDialog *trg_file_rename_dialog_new(TrgMainWindow * win,
                                                TrgClient * client,
                                                TrgFilesTreeView *
                                                ttv);

G_END_DECLS
#endif                          /* TRG_FILE_RENAME_DIALOG_H_ */
