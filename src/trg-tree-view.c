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

#include "trg-tree-view.h"
#include "trg-cell-renderer-speed.h"
#include "trg-cell-renderer-size.h"
#include "trg-cell-renderer-ratio.h"
#include "trg-cell-renderer-eta.h"

G_DEFINE_TYPE(TrgTreeView, trg_tree_view, GTK_TYPE_TREE_VIEW)

void trg_tree_view_std_column_setup(GtkTreeViewColumn * column, int index,
				    int width)
{
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, index);

    if (width > 0) {
	gtk_tree_view_column_set_sizing(column,
					GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, width);
    }
}

static void
trg_tree_view_class_init(TrgTreeViewClass * klass G_GNUC_UNUSED)
{
}

static void trg_tree_view_init(TrgTreeView * tv)
{
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(tv), TRUE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(tv), TRUE);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tv), TRUE);
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection
				(GTK_TREE_VIEW(tv)),
				GTK_SELECTION_MULTIPLE);

    gtk_widget_set_sensitive(GTK_WIDGET(tv), FALSE);
}

void trg_tree_view_add_size_column(TrgTreeView * tv, char *title,
				   int index, int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = trg_cell_renderer_size_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "size-value",
						      index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

void trg_tree_view_add_eta_column(TrgTreeView * tv, char *title, int index,
				  int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = trg_cell_renderer_eta_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "eta-value",
						      index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

void trg_tree_view_add_prog_column(TrgTreeView * tv,
				   gchar * title, gint index, gint width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_progress_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "value", index,
						      NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

void trg_tree_view_add_speed_column(TrgTreeView * tv, char *title,
				    int index, int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = trg_cell_renderer_speed_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "speed-value",
						      index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

void trg_tree_view_add_ratio_column(TrgTreeView * tv, char *title,
				    int index, int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = trg_cell_renderer_ratio_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "ratio-value",
						      index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

GtkCellRenderer *trg_tree_view_add_column_fixed_width(TrgTreeView * tv, char *title,
					  int index, int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "text", index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);

    return renderer;
}

void
trg_tree_view_add_pixbuf_text_column(TrgTreeView * tv,
				     int iconIndex,
				     int nameIndex,
				     gchar * text, int width)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    column = gtk_tree_view_column_new();

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(column), text);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, "stock-id",
					iconIndex, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer, "text",
					nameIndex, NULL);

    trg_tree_view_std_column_setup(column, nameIndex, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

GtkWidget *trg_tree_view_new(void)
{
    return GTK_WIDGET(g_object_new(TRG_TYPE_TREE_VIEW, NULL));
}
