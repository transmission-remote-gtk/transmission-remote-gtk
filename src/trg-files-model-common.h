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

#ifndef TRG_FILES_TREE_MODEL_COMMON_H_
#define TRG_FILES_TREE_MODEL_COMMON_H_

void trg_files_tree_model_propogate_change_up(GtkTreeModel *model, GtkTreeIter *iter, gint column,
                                              gint new_value);
void trg_files_tree_model_set_subtree(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                                      gint column, gint new_value);
void trg_files_tree_model_set_priority(GtkTreeView *tv, gint column, gint new_value);
void trg_files_model_set_wanted(GtkTreeView *tv, gint column, gint new_value);
gboolean trg_files_model_update_parent_size(GtkTreeModel *model, GtkTreePath *path,
                                            GtkTreeIter *iter, gpointer data);
void trg_files_model_update_parents(GtkTreeModel *model, GtkTreeIter *iter, gint size_column);

#endif /* TRG_FILES_TREE_MODEL_COMMON_H_ */
