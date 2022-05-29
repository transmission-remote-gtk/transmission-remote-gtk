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

#ifndef TRG_FILES_TREE_VIEW_COMMON_H_
#define TRG_FILES_TREE_VIEW_COMMON_H_

gboolean trg_files_tree_view_onViewButtonPressed(GtkWidget *w, GdkEventButton *event, gint pri_id,
                                                 gint enabled_id, GCallback rename_cb,
                                                 GCallback low_cb, GCallback normal_cb,
                                                 GCallback high_cb, GCallback wanted_cb,
                                                 GCallback unwanted_cb, gpointer gdata);

gboolean trg_files_tree_view_viewOnPopupMenu(GtkWidget *treeview, GCallback rename_cb,
                                             GCallback low_cb, GCallback normal_cb,
                                             GCallback high_cb, GCallback wanted_cb,
                                             GCallback unwanted_cb, gpointer userdata);

#endif /* TRG_FILES_TREE_VIEW_COMMON_H_ */
