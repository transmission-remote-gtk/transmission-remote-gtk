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

#ifndef _TRG_TREE_VIEW_H_
#define _TRG_TREE_VIEW_H_

#include <glib-object.h>

G_BEGIN_DECLS
#define TRG_TYPE_TREE_VIEW trg_tree_view_get_type()
#define TRG_TREE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_TREE_VIEW, TrgTreeView))
#define TRG_TREE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_TREE_VIEW, TrgTreeViewClass))
#define TRG_IS_TREE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_TREE_VIEW))
#define TRG_IS_TREE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_TREE_VIEW))
#define TRG_TREE_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_TREE_VIEW, TrgTreeViewClass))
    typedef struct {
    GtkTreeView parent;
} TrgTreeView;

typedef struct {
    GtkTreeViewClass parent_class;
} TrgTreeViewClass;

GType trg_tree_view_get_type(void);

GtkWidget *trg_tree_view_new(void);

G_END_DECLS
#define trg_tree_view_add_column(tv, title, index) trg_tree_view_add_column_fixed_width(tv, title, index, -1)

GList *trg_tree_view_get_selected_refs_list(GtkTreeView *tv);

GtkCellRenderer * trg_tree_view_add_column_fixed_width(TrgTreeView *
                                                           treeview,
                                                           char *title,
                                                           int index,
                                                           int width);

void trg_tree_view_add_pixbuf_text_column(TrgTreeView *
                                          treeview,
                                          int iconIndex,
                                          int nameIndex,
                                          gchar * text, int width);

void trg_tree_view_add_speed_column(TrgTreeView * tv, char *title,
                                    int index, int width);
void trg_tree_view_add_size_column(TrgTreeView * tv, char *title,
                                   int index, int width);
void trg_tree_view_add_prog_column(TrgTreeView * tv, gchar * title,
                                   gint index, gint width);
void trg_tree_view_add_ratio_column(TrgTreeView * tv, char *title,
                                    int index, int width);
void trg_tree_view_add_eta_column(TrgTreeView * tv, char *title, int index,
                                  int width);
void trg_tree_view_std_column_setup(GtkTreeViewColumn * column, int index,
                                    int width);

#endif                          /* _TRG_TREE_VIEW_H_ */
