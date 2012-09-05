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

#ifndef _TRG_TREE_VIEW_H_
#define _TRG_TREE_VIEW_H_

#include <glib-object.h>

#include "trg-prefs.h"

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

G_END_DECLS GList *trg_tree_view_get_selected_refs_list(GtkTreeView * tv);

enum {
    TRG_COLTYPE_STOCKICONTEXT,
    TRG_COLTYPE_FILEICONTEXT,
    TRG_COLTYPE_WANTED,
    TRG_COLTYPE_TEXT,
    TRG_COLTYPE_SIZE,
    TRG_COLTYPE_RATIO,
    TRG_COLTYPE_EPOCH,
    TRG_COLTYPE_SPEED,
    TRG_COLTYPE_ETA,
    TRG_COLTYPE_PROG,
    TRG_COLTYPE_PRIO,
    TRG_COLTYPE_NUMGTZERO,
    TRG_COLTYPE_NUMGTEQZERO
} TrgColumnType;

typedef struct {
    gint model_column;
    gint model_column_extra;
    gchar *header;
    gchar *id;
    guint flags;
    guint type;
    GtkCellRenderer *customRenderer;
    GtkTreeViewColumn **out;
} trg_column_description;

#define TRG_COLUMN_DEFAULT             0x00
#define TRG_COLUMN_SHOWING             (1 << 0) /* 0x01 */
#define TRG_COLUMN_UNREMOVABLE         (1 << 1) /* 0x02 */
#define TRG_COLUMN_EXTRA               (1 << 2) /* 0x04 */
#define TRG_COLUMN_HIDE_FROM_TOP_MENU  (1 << 3) /* 0x08 */

#define TRG_TREE_VIEW_PERSIST_SORT	   (1 << 0)
#define TRG_TREE_VIEW_PERSIST_LAYOUT   (1 << 1)
#define TRG_TREE_VIEW_SORTABLE_PARENT  (1 << 2)

trg_column_description *trg_tree_view_reg_column(TrgTreeView * tv,
                                                 gint type,
                                                 gint model_column,
                                                 const gchar * header,
                                                 const gchar * id,
                                                 guint flags);
void trg_tree_view_setup_columns(TrgTreeView * tv);
void trg_tree_view_set_prefs(TrgTreeView * tv, TrgPrefs * prefs);
void trg_tree_view_persist(TrgTreeView * tv, guint flags);
void trg_tree_view_remove_all_columns(TrgTreeView * tv);
void trg_tree_view_restore_sort(TrgTreeView * tv, guint flags);
GtkWidget *trg_tree_view_sort_menu(TrgTreeView * tv, const gchar * label);
gboolean trg_tree_view_is_column_showing(TrgTreeView * tv, gint index);

#endif                          /* _TRG_TREE_VIEW_H_ */
