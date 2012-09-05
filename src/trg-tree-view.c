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

#include <stdlib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <glib/gi18n.h>

#include "trg-prefs.h"
#include "trg-tree-view.h"
#include "trg-cell-renderer-speed.h"
#include "trg-cell-renderer-size.h"
#include "trg-cell-renderer-ratio.h"
#include "trg-cell-renderer-wanted.h"
#include "trg-cell-renderer-eta.h"
#include "trg-cell-renderer-epoch.h"
#include "trg-cell-renderer-priority.h"
#include "trg-cell-renderer-numgteqthan.h"
#include "trg-cell-renderer-file-icon.h"

/* A subclass of GtkTreeView which allows the user to change column visibility
 * by right clicking on any column for a menu to hide the clicked column, or
 * insert any hidden column after.
 *
 * This class persists these choices to TrgPrefs, and restores them when it is
 * initialised. Column widths are also saved/restored.
 *
 * All the columns must be preregistered so it knows what model column,
 * renderers etc to use if it should be created, and what columns are available.
 */

enum {
    PROP_0, PROP_PREFS, PROP_CONFIGID
};

G_DEFINE_TYPE(TrgTreeView, trg_tree_view, GTK_TYPE_TREE_VIEW)
#define TRG_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TREE_VIEW, TrgTreeViewPrivate))
typedef struct _TrgTreeViewPrivate TrgTreeViewPrivate;

struct _TrgTreeViewPrivate {
    GList *columns;
    TrgPrefs *prefs;
    gchar *configId;
};

#define GDATA_KEY_COLUMN_DESC "column-desc"

gboolean trg_tree_view_is_column_showing(TrgTreeView * tv, gint index)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);

    GList *li;
    for (li = priv->columns; li; li = g_list_next(li)) {
        trg_column_description *cd = (trg_column_description *) li->data;
        if (cd->model_column == index) {
            if (cd->flags & TRG_COLUMN_SHOWING)
                return TRUE;
            else
                break;
        }
    }

    return FALSE;
}

static void
trg_tree_view_get_property(GObject * object, guint property_id,
                           GValue * value, GParamSpec * pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void
trg_tree_view_set_property(GObject * object, guint property_id,
                           const GValue * value, GParamSpec * pspec)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_PREFS:
        priv->prefs = g_value_get_object(value);
        break;
    case PROP_CONFIGID:
        g_free(priv->configId);
        priv->configId = g_value_dup_string(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static GObject *trg_tree_view_constructor(GType type,
                                          guint n_construct_properties,
                                          GObjectConstructParam *
                                          construct_params)
{
    GObject *obj = G_OBJECT_CLASS
        (trg_tree_view_parent_class)->constructor(type,
                                                  n_construct_properties,
                                                  construct_params);

    return obj;
}

static JsonObject *trg_prefs_get_tree_view_props(TrgTreeView * tv)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    JsonObject *root = trg_prefs_get_root(priv->prefs);
    const gchar *className = priv->configId
        && strlen(priv->configId) >
        0 ? priv->configId : G_OBJECT_TYPE_NAME(tv);
    JsonObject *obj;
    JsonObject *tvProps = NULL;

    if (!json_object_has_member(root, TRG_PREFS_KEY_TREE_VIEWS)) {
        obj = json_object_new();
        json_object_set_object_member(root, TRG_PREFS_KEY_TREE_VIEWS, obj);
    } else {
        obj =
            json_object_get_object_member(root, TRG_PREFS_KEY_TREE_VIEWS);
    }

    if (!json_object_has_member(obj, className)) {
        tvProps = json_object_new();
        json_object_set_object_member(obj, className, tvProps);
    } else {
        tvProps = json_object_get_object_member(obj, className);
    }

    return tvProps;
}

static void trg_tree_view_add_column_after(TrgTreeView * tv,
                                           trg_column_description * desc,
                                           gint64 width,
                                           GtkTreeViewColumn * after_col);

trg_column_description *trg_tree_view_reg_column(TrgTreeView * tv,
                                                 gint type,
                                                 gint model_column,
                                                 const gchar * header,
                                                 const gchar * id,
                                                 guint flags)
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
                                                         const gchar * id)
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

static void
trg_tree_view_hide_column(GtkWidget * w, GtkTreeViewColumn * col)
{
    trg_column_description *desc = g_object_get_data(G_OBJECT(col),
                                                     GDATA_KEY_COLUMN_DESC);
    GtkWidget *tv = gtk_tree_view_column_get_tree_view(col);
    desc->flags &= ~TRG_COLUMN_SHOWING;
    gtk_tree_view_remove_column(GTK_TREE_VIEW(tv), col);
}

static void
trg_tree_view_add_column(TrgTreeView * tv,
                         trg_column_description * desc, gint64 width)
{
    trg_tree_view_add_column_after(tv, desc, width, NULL);
}

static void
trg_tree_view_user_add_column_cb(GtkWidget * w,
                                 trg_column_description * desc)
{
    GtkTreeViewColumn *col = g_object_get_data(G_OBJECT(w), "parent-col");
    TrgTreeView *tv =
        TRG_TREE_VIEW(gtk_tree_view_column_get_tree_view(col));

    trg_tree_view_add_column_after(tv, desc, -1, col);
}

static void trg_tree_view_sort_menu_item_toggled(GtkCheckMenuItem * w,
                                                 gpointer data)
{
    GtkTreeSortable *model = GTK_TREE_SORTABLE(data);
    trg_column_description *desc =
        (trg_column_description *) g_object_get_data(G_OBJECT(w),
                                                     GDATA_KEY_COLUMN_DESC);

    if (gtk_check_menu_item_get_active(w)) {
        GtkSortType sortType;
        gtk_tree_sortable_get_sort_column_id(model, NULL, &sortType);
        gtk_tree_sortable_set_sort_column_id(model, desc->model_column,
                                             sortType);
    }
}

static void trg_tree_view_sort_menu_type_toggled(GtkCheckMenuItem * w,
                                                 gpointer data)
{
    GtkTreeSortable *model = GTK_TREE_SORTABLE(data);

    if (gtk_check_menu_item_get_active(w)) {
        gint sortColumn;
        gint sortType =
            GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "sort-type"));
        gtk_tree_sortable_get_sort_column_id(model, &sortColumn, NULL);
        gtk_tree_sortable_set_sort_column_id(model, sortColumn, sortType);
    }
}


GtkWidget *trg_tree_view_sort_menu(TrgTreeView * tv, const gchar * label)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    GtkWidget *item = gtk_menu_item_new_with_mnemonic(label);
    GtkTreeModel *treeViewModel =
        gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    GtkTreeSortable *sortableModel =
        GTK_TREE_SORTABLE(gtk_tree_model_filter_get_model
                          (GTK_TREE_MODEL_FILTER(treeViewModel)));
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *b;
    GList *li;
    gint sort;
    GtkSortType sortType;
    GSList *group = NULL;

    gtk_tree_sortable_get_sort_column_id(sortableModel, &sort, &sortType);

    b = gtk_radio_menu_item_new_with_label(group, _("Ascending"));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(b),
                                   sortType == GTK_SORT_ASCENDING);
    g_object_set_data(G_OBJECT(b), "sort-type",
                      GINT_TO_POINTER(GTK_SORT_ASCENDING));
    g_signal_connect(b, "toggled",
                     G_CALLBACK(trg_tree_view_sort_menu_type_toggled),
                     sortableModel);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), b);
    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(b));
    b = gtk_radio_menu_item_new_with_label(group, _("Descending"));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(b),
                                   sortType == GTK_SORT_DESCENDING);
    g_object_set_data(G_OBJECT(b), "sort-type",
                      GINT_TO_POINTER(GTK_SORT_DESCENDING));
    g_signal_connect(b, "toggled",
                     G_CALLBACK(trg_tree_view_sort_menu_type_toggled),
                     sortableModel);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), b);

    group = NULL;

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());

    for (li = priv->columns; li; li = g_list_next(li)) {
        trg_column_description *desc = (trg_column_description *) li->data;
        if (!(desc->flags & TRG_COLUMN_HIDE_FROM_TOP_MENU)) {
            b = gtk_radio_menu_item_new_with_label(group, desc->header);
            group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(b));

            if (desc->model_column == sort)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(b),
                                               TRUE);

            g_object_set_data(G_OBJECT(b), GDATA_KEY_COLUMN_DESC, desc);
            g_signal_connect(b, "toggled",
                             G_CALLBACK
                             (trg_tree_view_sort_menu_item_toggled),
                             sortableModel);

            gtk_menu_shell_append(GTK_MENU_SHELL(menu), b);
        }
    }

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);

    return item;
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

    desc = g_object_get_data(G_OBJECT(column), GDATA_KEY_COLUMN_DESC);
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
col_onButtonPressed(GtkButton * button,
                    GdkEventButton * event, GtkTreeViewColumn * col)
{
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        view_popup_menu(button, event, col);
        return TRUE;
    }

    return FALSE;
}

static GtkTreeViewColumn
    * trg_tree_view_icontext_column_new(trg_column_description * desc,
                                        gchar * renderer_property)
{
    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();

    gtk_tree_view_column_set_title(column, desc->header);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer,
                                        renderer_property,
                                        desc->model_column_extra, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer, "text",
                                        desc->model_column, NULL);

    return column;
}

static GtkTreeViewColumn
    * trg_tree_view_fileicontext_column_new(trg_column_description * desc)
{
    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    GtkCellRenderer *renderer = trg_cell_renderer_file_icon_new();

    gtk_tree_view_column_set_title(column, desc->header);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer,
                                        "file-id",
                                        desc->model_column_extra,
                                        "file-name", desc->model_column,
                                        NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer, "text",
                                        desc->model_column, NULL);

    return column;
}

static void
trg_tree_view_add_column_after(TrgTreeView * tv,
                               trg_column_description * desc,
                               gint64 width, GtkTreeViewColumn * after_col)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column = NULL;

    switch (desc->type) {
    case TRG_COLTYPE_TEXT:
        renderer =
            desc->customRenderer ? desc->customRenderer :
            gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer, "text",
                                                          desc->
                                                          model_column,
                                                          NULL);

        break;
    case TRG_COLTYPE_SPEED:
        renderer = trg_cell_renderer_speed_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "speed-value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_EPOCH:
        renderer = trg_cell_renderer_epoch_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "epoch-value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_ETA:
        renderer = trg_cell_renderer_eta_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "eta-value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_SIZE:
        renderer = trg_cell_renderer_size_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "size-value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_PROG:
        renderer = gtk_cell_renderer_progress_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_RATIO:
        renderer = trg_cell_renderer_ratio_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "ratio-value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_WANTED:
        column = gtk_tree_view_column_new();
        renderer = trg_cell_renderer_wanted_new();
        /*gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(renderer), 0.5f,
           0.0); */
        gtk_tree_view_column_set_title(column, desc->header);
        gtk_tree_view_column_pack_start(column, renderer, TRUE);
        gtk_tree_view_column_set_attributes(column, renderer,
                                            "wanted-value",
                                            desc->model_column, NULL);
        break;
    case TRG_COLTYPE_STOCKICONTEXT:
        column = trg_tree_view_icontext_column_new(desc, "stock-id");
        break;
    case TRG_COLTYPE_FILEICONTEXT:
        column = trg_tree_view_fileicontext_column_new(desc);
        break;
    case TRG_COLTYPE_PRIO:
        renderer = trg_cell_renderer_priority_new();
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "priority-value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_NUMGTZERO:
        renderer = trg_cell_renderer_numgteqthan_new(1);
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    case TRG_COLTYPE_NUMGTEQZERO:
        renderer = trg_cell_renderer_numgteqthan_new(0);
        column = gtk_tree_view_column_new_with_attributes(desc->header,
                                                          renderer,
                                                          "value",
                                                          desc->
                                                          model_column,
                                                          NULL);
        break;
    default:
        g_critical("unknown TrgTreeView column");
        return;
    }

    gtk_tree_view_column_set_min_width(column, 0);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, desc->model_column);

    if (width > 0) {
        gtk_tree_view_column_set_sizing(column,
                                        GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width(column, width);
    }

    g_object_set_data(G_OBJECT(column), GDATA_KEY_COLUMN_DESC, desc);

    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);

    if (after_col)
        gtk_tree_view_move_column_after(GTK_TREE_VIEW(tv), column,
                                        after_col);

#if GTK_CHECK_VERSION( 3,0,0 )
    g_signal_connect(gtk_tree_view_column_get_button(column),
                     "button-press-event", G_CALLBACK(col_onButtonPressed),
                     column);
#else
    g_signal_connect(column->button, "button-press-event",
                     G_CALLBACK(col_onButtonPressed), column);
#endif

    if (desc->out)
        *(desc->out) = column;

    desc->flags |= TRG_COLUMN_SHOWING;
}

void trg_tree_view_remove_all_columns(TrgTreeView * tv)
{
    GtkTreeView *gtv = GTK_TREE_VIEW(tv);
    GList *cols = gtk_tree_view_get_columns(gtv);
    GList *li;
    for (li = cols; li; li = g_list_next(li)) {
        gtk_tree_view_remove_column(gtv, GTK_TREE_VIEW_COLUMN(li->data));
    }
    g_list_free(cols);
}

void trg_tree_view_persist(TrgTreeView * tv, guint flags)
{
    JsonObject *props = trg_prefs_get_tree_view_props(tv);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    GList *cols, *li;
    JsonArray *widths, *columns;
    gint sort_column_id;
    GtkSortType sort_type;

    if (flags & TRG_TREE_VIEW_PERSIST_SORT) {
        gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE
                                             ((flags &
                                               TRG_TREE_VIEW_SORTABLE_PARENT)
                                              ?
                                              gtk_tree_model_filter_get_model
                                              (GTK_TREE_MODEL_FILTER
                                               (model)) : model),
                                             &sort_column_id, &sort_type);

        if (json_object_has_member(props, TRG_PREFS_KEY_TV_SORT_COL))
            json_object_remove_member(props, TRG_PREFS_KEY_TV_SORT_COL);

        if (json_object_has_member(props, TRG_PREFS_KEY_TV_SORT_TYPE))
            json_object_remove_member(props, TRG_PREFS_KEY_TV_SORT_TYPE);

        json_object_set_int_member(props, TRG_PREFS_KEY_TV_SORT_COL,
                                   (gint64) sort_column_id);
        json_object_set_int_member(props, TRG_PREFS_KEY_TV_SORT_TYPE,
                                   (gint64) sort_type);
    }

    if (flags & TRG_TREE_VIEW_PERSIST_LAYOUT) {
        cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(tv));

        if (json_object_has_member(props, TRG_PREFS_KEY_TV_WIDTHS))
            json_object_remove_member(props, TRG_PREFS_KEY_TV_WIDTHS);

        widths = json_array_new();
        json_object_set_array_member(props, TRG_PREFS_KEY_TV_WIDTHS,
                                     widths);

        if (json_object_has_member(props, TRG_PREFS_KEY_TV_COLUMNS))
            json_object_remove_member(props, TRG_PREFS_KEY_TV_COLUMNS);

        columns = json_array_new();
        json_object_set_array_member(props, TRG_PREFS_KEY_TV_COLUMNS,
                                     columns);

        for (li = cols; li; li = g_list_next(li)) {
            GtkTreeViewColumn *col = (GtkTreeViewColumn *) li->data;
            trg_column_description *desc =
                g_object_get_data(G_OBJECT(li->data),
                                  GDATA_KEY_COLUMN_DESC);

            json_array_add_string_element(columns, desc->id);
            json_array_add_int_element(widths,
                                       gtk_tree_view_column_get_width
                                       (col));
        }

        g_list_free(cols);
    }
}

void trg_tree_view_restore_sort(TrgTreeView * tv, guint flags)
{
    JsonObject *props = trg_prefs_get_tree_view_props(tv);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));

    if (json_object_has_member(props, TRG_PREFS_KEY_TV_SORT_COL)
        && json_object_has_member(props, TRG_PREFS_KEY_TV_SORT_TYPE)) {
        gint64 sort_col = json_object_get_int_member(props,
                                                     TRG_PREFS_KEY_TV_SORT_COL);
        gint64 sort_type = json_object_get_int_member(props,
                                                      TRG_PREFS_KEY_TV_SORT_TYPE);
        gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE
                                             ((flags &
                                               TRG_TREE_VIEW_SORTABLE_PARENT)
                                              ?
                                              gtk_tree_model_filter_get_model
                                              (GTK_TREE_MODEL_FILTER
                                               (model)) : model), sort_col,
                                             (GtkSortType) sort_type);

    }
}

void trg_tree_view_set_prefs(TrgTreeView * tv, TrgPrefs * prefs)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    priv->prefs = prefs;
}

void trg_tree_view_setup_columns(TrgTreeView * tv)
{
    TrgTreeViewPrivate *priv = TRG_TREE_VIEW_GET_PRIVATE(tv);
    JsonObject *props = trg_prefs_get_tree_view_props(tv);
    GList *columns, *widths, *cli, *wli;

    if (!json_object_has_member(props, TRG_PREFS_KEY_TV_COLUMNS)
        || !json_object_has_member(props, TRG_PREFS_KEY_TV_WIDTHS)) {
        GList *li;
        for (li = priv->columns; li; li = g_list_next(li)) {
            trg_column_description *desc =
                (trg_column_description *) li->data;
            if (desc && !(desc->flags & TRG_COLUMN_EXTRA))
                trg_tree_view_add_column(tv, desc, -1);
        }
        return;
    }

    columns =
        json_array_get_elements(json_object_get_array_member
                                (props, TRG_PREFS_KEY_TV_COLUMNS));
    widths =
        json_array_get_elements(json_object_get_array_member
                                (props, TRG_PREFS_KEY_TV_WIDTHS));

    for (cli = columns, wli = widths; cli && wli;
         cli = g_list_next(cli), wli = g_list_next(wli)) {
        trg_column_description *desc = trg_tree_view_find_column(tv,
                                                                 json_node_get_string
                                                                 ((JsonNode
                                                                   *)
                                                                  cli->
                                                                  data));
        if (desc) {
            gint64 width = json_node_get_int((JsonNode *) wli->data);
            trg_tree_view_add_column(tv, desc, width);
        }
    }

    g_list_free(columns);
    g_list_free(widths);
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

static void trg_tree_view_class_init(TrgTreeViewClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgTreeViewPrivate));

    object_class->get_property = trg_tree_view_get_property;
    object_class->set_property = trg_tree_view_set_property;
    object_class->constructor = trg_tree_view_constructor;

    g_object_class_install_property(object_class,
                                    PROP_PREFS,
                                    g_param_spec_object("prefs",
                                                        "Trg Prefs",
                                                        "Trg Prefs",
                                                        TRG_TYPE_PREFS,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_CONFIGID,
                                    g_param_spec_string
                                    ("config-id",
                                     "config-id",
                                     "config-id",
                                     NULL,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));
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
