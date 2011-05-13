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
#include <glib/gi18n.h>

#include "trg-tree-view.h"
#include "trg-cell-renderer-speed.h"
#include "trg-cell-renderer-size.h"
#include "trg-cell-renderer-ratio.h"
#include "trg-cell-renderer-eta.h"
#include "trg-cell-renderer-epoch.h"
#include "trg-cell-renderer-wanted.h"
#include "trg-cell-renderer-priority.h"
#include "trg-cell-renderer-numgtzero.h"

G_DEFINE_TYPE(TrgTreeView, trg_tree_view, GTK_TYPE_TREE_VIEW)
#define TRG_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TREE_VIEW, TrgTreeViewPrivate))
typedef struct _TrgTreeViewPrivate TrgTreeViewPrivate;

struct _TrgTreeViewPrivate {
    GList *columns;
};

static void trg_tree_view_add_column_after(TrgTreeView * tv,
                                           trg_column_description * desc,
                                           gchar ** widths, gint i,
                                           GtkTreeViewColumn * after_col);

trg_column_description *trg_tree_view_reg_column(TrgTreeView * tv,
                                                 gint type,
                                                 gint model_column,
                                                 gchar * header,
                                                 gchar * id, gint flags)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    trg_column_description *desc = g_new0(trg_column_description, 1);

    desc->type = type;
    desc->model_column = model_column;
    desc->header = g_strdup(header);
    desc->id = g_strdup(id);
    desc->flags = flags;

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

static void trg_tree_view_hide_column(GtkWidget * w,
                                      GtkTreeViewColumn * col)
{
    trg_column_description *desc =
        g_object_get_data(G_OBJECT(col), "column-desc");
    GtkWidget *tv = gtk_tree_view_column_get_tree_view(col);
    desc->flags &= ~TRG_COLUMN_SHOWING;
    gtk_tree_view_remove_column(GTK_TREE_VIEW(tv), col);
}

static void trg_tree_view_add_column(TrgTreeView * tv,
                                     trg_column_description * desc,
                                     gchar ** widths, gint i)
{
    trg_tree_view_add_column_after(tv, desc, widths, i, NULL);
}

static void trg_tree_view_user_add_column_cb(GtkWidget * w,
                                             trg_column_description * desc)
{
    GtkTreeViewColumn *col = g_object_get_data(G_OBJECT(w), "parent-col");
    TrgTreeView *tv =
        TRG_TREE_VIEW(gtk_tree_view_column_get_tree_view(col));

    trg_tree_view_add_column_after(tv, desc, NULL, -1, col);
}

static void
view_popup_menu(GtkButton * button, GdkEventButton * event,
                GtkTreeViewColumn * column)
{
    GtkWidget *tv = gtk_tree_view_column_get_tree_view(column);
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    GtkWidget *menu, *menuitem;
    trg_column_description *desc;
    GList *li;

    menu = gtk_menu_new();

    desc = g_object_get_data(G_OBJECT(column), "column-desc");
    menuitem = gtk_check_menu_item_new_with_label(desc->header);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);
    g_signal_connect(menuitem, "activate",
                     G_CALLBACK(trg_tree_view_hide_column), column);
    gtk_widget_set_sensitive(menuitem,
                             !(desc->flags & TRG_COLUMN_UNREMOVABLE));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    for (li = priv->columns; li; li = g_list_next(li)) {
        trg_column_description *desc = (trg_column_description *) li->data;
        if (!(desc->flags & TRG_COLUMN_SHOWING)) {
            menuitem = gtk_check_menu_item_new_with_label(desc->header);
            g_object_set_data(G_OBJECT(menuitem), "parent-col", column);
            g_signal_connect(menuitem, "activate",
                             G_CALLBACK(trg_tree_view_user_add_column_cb),
                             desc);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        }
    }
    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent *) event));
}

static gboolean
col_onButtonPressed(GtkButton * button, GdkEventButton * event,
                    GtkTreeViewColumn * col)
{
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        view_popup_menu(button, event, col);
        return TRUE;
    }

    return FALSE;
}

static void trg_tree_view_add_column_after(TrgTreeView * tv,
                                           trg_column_description * desc,
                                           gchar ** widths, gint i,
                                           GtkTreeViewColumn * after_col)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column = NULL;

    switch (desc->type) {
    case TRG_COLTYPE_TEXT:
        renderer =
            desc->customRenderer ? desc->customRenderer :
            gtk_cell_renderer_text_new();
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
    case TRG_COLTYPE_NUMGTZERO:
        renderer = trg_cell_renderer_numgtzero_new();
        column =
            gtk_tree_view_column_new_with_attributes(desc->header,
                                                     renderer,
                                                     "value",
                                                     desc->model_column,
                                                     NULL);
        break;
    }

    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, desc->model_column);

    if (!widths && desc->defaultWidth > 0) {
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

    if (after_col)
        gtk_tree_view_move_column_after(GTK_TREE_VIEW(tv), column,
                                        after_col);

    g_signal_connect(column->button, "button-press-event",
                     G_CALLBACK(col_onButtonPressed), column);

    if (desc->out)
        *(desc->out) = column;

    desc->flags |= TRG_COLUMN_SHOWING;
}

void trg_tree_view_persist(TrgTreeView * tv)
{
    GConfClient *gcc = gconf_client_get_default();
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    GList *cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(tv));
    gint n_cols = g_list_length(cols);
    gint sort_column_id;
    GtkSortType sort_type;
    const gchar *tree_view_name = G_OBJECT_TYPE_NAME(tv);
    gchar *cols_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-columns",
                        tree_view_name);
    gchar *widths_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-widths",
                        tree_view_name);
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

    if (gtk_tree_sortable_get_sort_column_id
        (GTK_TREE_SORTABLE(model), &sort_column_id, &sort_type)) {
        gchar *sort_col_key =
            g_strdup_printf("/apps/transmission-remote-gtk/%s-sort_col",
                            tree_view_name);
        gchar *sort_type_key =
            g_strdup_printf("/apps/transmission-remote-gtk/%s-sort_type",
                            tree_view_name);
        gconf_client_set_int(gcc, sort_col_key, sort_column_id, NULL);
        gconf_client_set_int(gcc, sort_type_key, (gint) sort_type, NULL);
        g_free(sort_type_key);
        g_free(sort_col_key);
    }

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

void trg_tree_view_restore_sort(TrgTreeView * tv)
{
    GConfClient *gcc = gconf_client_get_default();
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    gchar *sort_col_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-sort_col",
                        G_OBJECT_TYPE_NAME(tv));
    gchar *sort_type_key =
        g_strdup_printf("/apps/transmission-remote-gtk/%s-sort_type",
                        G_OBJECT_TYPE_NAME(tv));
    GConfValue *sort_col_gv =
        gconf_client_get_without_default(gcc, sort_col_key, NULL);
    GConfValue *sort_type_gv =
        gconf_client_get_without_default(gcc, sort_type_key, NULL);
    if (sort_col_gv) {
        gint sort_col_value = gconf_value_get_int(sort_col_gv);
        GtkSortType sort_type_value = GTK_SORT_ASCENDING;
        if (sort_type_gv) {
            sort_type_value =
                (GtkSortType) gconf_value_get_int(sort_type_gv);
            gconf_value_free(sort_type_gv);
        }
        gconf_value_free(sort_col_gv);
        gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                                             sort_col_value,
                                             sort_type_value);
    }
    g_free(sort_col_key);
    g_free(sort_type_key);
    g_object_unref(gcc);
}

void trg_tree_view_setup_columns(TrgTreeView * tv)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    gchar **columns = trg_gconf_get_csv(tv, "columns");
    GList *li;
    int i;
    trg_column_description *desc;

    if (columns) {
        gchar **widths = trg_gconf_get_csv(tv, "widths");
        for (i = 0; columns[i]; i++) {
            trg_column_description *desc =
                trg_tree_view_find_column(tv, columns[i]);
            if (desc)
                trg_tree_view_add_column(tv, desc, widths, i);
        }
        g_strfreev(columns);
        if (widths)
            g_strfreev(widths);
    } else {
        for (li = priv->columns; li; li = g_list_next(li)) {
            desc = (trg_column_description *) li->data;
            if (desc && !(desc->flags & TRG_COLUMN_EXTRA))
                trg_tree_view_add_column(tv, desc, NULL, -1);
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
