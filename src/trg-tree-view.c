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

#include <stdlib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include "trg-tree-view.h"
#include "trg-cell-renderer-speed.h"
#include "trg-cell-renderer-size.h"
#include "trg-cell-renderer-ratio.h"
#include "trg-cell-renderer-eta.h"
#include "trg-cell-renderer-epoch.h"
#include "trg-cell-renderer-wanted.h"
#include "trg-cell-renderer-priority.h"

G_DEFINE_TYPE(TrgTreeView, trg_tree_view, GTK_TYPE_TREE_VIEW)
#define TRG_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TREE_VIEW, TrgTreeViewPrivate))
typedef struct _TrgTreeViewPrivate TrgTreeViewPrivate;

struct _TrgTreeViewPrivate {
    GList *columns;
};

trg_column_description *trg_tree_view_reg_column(TrgTreeView * tv,
                                                 gint type,
                                                 gint model_column,
                                                 gchar * header,
                                                 gchar * id, gint show)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    trg_column_description *desc = g_new0(trg_column_description, 1);

    desc->type = type;
    desc->model_column = model_column;
    desc->header = g_strdup(header);
    desc->id = g_strdup(id);
    desc->show = show;

    priv->columns = g_list_append(priv->columns, desc);

    return desc;
}

static trg_column_description *trg_tree_view_find_column(TrgTreeView * tv,
                                                         gchar * id)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    GList *li;
    trg_column_description *desc;

    for (li = priv->columns; li; li = g_list_next(li)) {
        desc = (trg_column_description *) li->data;
        if (!g_strcmp0(desc->id, id))
            return desc;
    }

    return NULL;
}

static gchar **trg_gconf_get_csv(TrgTreeView * tv, gchar * key)
{
    gchar **ret = NULL;
    GConfClient *gcc = gconf_client_get_default();
    gchar *gconf_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-%s",
                        G_OBJECT_TYPE_NAME(tv), key);
    gchar *gconf_value = gconf_client_get_string(gcc, gconf_key, NULL);

    if (gconf_value) {
        ret = g_strsplit(gconf_value, ",", -1);
        g_free(gconf_value);
    }

    g_free(gconf_key);
    g_object_unref(gcc);

    return ret;
}

static void trg_tree_view_add_column(TrgTreeView * tv,
                                     trg_column_description * desc,
                                     gchar ** widths, gint i)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column = NULL;

    switch (desc->type) {
    case TRG_COLTYPE_TEXT:
        renderer =
            desc->customRenderer ? desc->
            customRenderer : gtk_cell_renderer_text_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer, "text",
                                                     desc->model_column,
                                                     NULL);

        break;
    case TRG_COLTYPE_SPEED:
        renderer = trg_cell_renderer_speed_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "speed-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_EPOCH:
        renderer = trg_cell_renderer_epoch_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "epoch-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_ETA:
        renderer = trg_cell_renderer_eta_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer, "eta-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_SIZE:
        renderer = trg_cell_renderer_size_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "size-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_PROG:
        renderer = gtk_cell_renderer_progress_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer, "value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_RATIO:
        renderer = trg_cell_renderer_ratio_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "ratio-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_ICONTEXT:
        column = gtk_tree_view_column_new();

        renderer = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_set_title(column, desc->header);
        gtk_tree_view_column_pack_start(column, renderer, FALSE);
        gtk_tree_view_column_set_attributes(column, renderer, "stock-id",
                                            desc->model_column_icon, NULL);

        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, renderer, TRUE);
        gtk_tree_view_column_set_attributes(column, renderer, "text",
                                            desc->model_column, NULL);
        break;
    case TRG_COLTYPE_WANT:
        renderer = trg_cell_renderer_wanted_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "wanted-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    case TRG_COLTYPE_PRIO:
        renderer = trg_cell_renderer_priority_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "priority-value",
                                                     desc->model_column,
                                                     NULL);
        break;
    }

    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, desc->model_column);

    if (i < 0 && desc->defaultWidth > 0) {
        gtk_tree_view_column_set_sizing(column,
                                        GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width(column, desc->defaultWidth);
    } else if (widths && i >= 0) {
        gchar *ws = widths[i];
        int w = atoi(ws);
        gtk_tree_view_column_set_sizing(column,
                                        GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width(column, w);
    }

    g_object_set_data(G_OBJECT(column), "column-desc", desc);

    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);

    if (desc->out)
        *(desc->out) = column;
}

void trg_tree_view_persist(TrgTreeView * tv)
{
    GConfClient *gcc = gconf_client_get_default();
    GList *cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(tv));
    gint n_cols = g_list_length(cols);
    gchar *cols_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-columns",
                        G_OBJECT_TYPE_NAME(tv));
    gchar *widths_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-widths",
                        G_OBJECT_TYPE_NAME(tv));
    gchar **cols_v = g_new0(gchar *, n_cols + 1);
    gchar **widths_v = g_new0(gchar *, n_cols + 1);
    gchar *widths_js, *cols_js;
    GList *li;
    int i = 0;

    for (li = cols; li; li = g_list_next(li)) {
        GtkTreeViewColumn *col = (GtkTreeViewColumn *) li->data;
        trg_column_description *desc =
            g_object_get_data(G_OBJECT(li->data), "column-desc");
        cols_v[i] = desc->id;
        widths_v[i] =
            g_strdup_printf("%d", gtk_tree_view_column_get_width(col));
        i++;
    }

    widths_js = g_strjoinv(",", widths_v);
    cols_js = g_strjoinv(",", cols_v);

    gconf_client_set_string(gcc, cols_key, cols_js, NULL);
    gconf_client_set_string(gcc, widths_key, widths_js, NULL);

    g_free(cols_key);
    g_free(widths_key);
    g_free(widths_js);
    g_free(cols_js);
    g_free(cols_v);
    g_strfreev(widths_v);
    g_list_free(cols);
    g_object_unref(gcc);
}

void trg_tree_view_setup_columns(TrgTreeView * tv)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    gchar **columns = trg_gconf_get_csv(tv, "columns");
    gchar **widths = trg_gconf_get_csv(tv, "widths");
    GList *li;
    int i;
    trg_column_description *desc;

    if (columns) {
        for (i = 0; columns[i]; i++) {
            trg_column_description *desc =
                trg_tree_view_find_column(tv, columns[i]);
            if (desc)
                trg_tree_view_add_column(tv, desc, widths, i);
        }
    } else {
        for (li = priv->columns; li; li = g_list_next(li)) {
            desc = (trg_column_description *) li->data;
            if (desc && desc->show != 0)
                trg_tree_view_add_column(tv, desc, widths, -1);
        }
    }
}

GList *trg_tree_view_get_selected_refs_list(GtkTreeView * tv)
{
    GtkTreeModel *model = gtk_tree_view_get_model(tv);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tv);
    GList *li, *selectionList;
    GList *refList = NULL;

    selectionList = gtk_tree_selection_get_selected_rows(selection, NULL);
    for (li = selectionList; li != NULL; li = g_list_next(li)) {
        GtkTreePath *path = (GtkTreePath *) li->data;
        GtkTreeRowReference *ref = gtk_tree_row_reference_new(model, path);
        gtk_tree_path_free(path);
        refList = g_list_append(refList, ref);
    }
    g_list_free(selectionList);

    return refList;
}

static void
trg_tree_view_class_init(TrgTreeViewClass * klass G_GNUC_UNUSED)
{
    g_type_class_add_private(klass, sizeof(TrgTreeViewPrivate));
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

GtkWidget *trg_tree_view_new(void)
{
    return GTK_WIDGET(g_object_new(TRG_TYPE_TREE_VIEW, NULL));
}
