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

#include "trg-sortable-filtered-model.h"

static void
trg_sortable_filtered_model_tree_sortable_init(GtkTreeSortableIface *iface);

/* TreeSortable interface */
static gboolean trg_sortable_filtered_model_sort_get_sort_column_id(
        GtkTreeSortable *sortable, gint *sort_column_id, GtkSortType *order);
static void trg_sortable_filtered_model_sort_set_sort_column_id(
        GtkTreeSortable *sortable, gint sort_column_id, GtkSortType order);
static void trg_sortable_filtered_model_sort_set_sort_func(
        GtkTreeSortable *sortable, gint sort_column_id,
        GtkTreeIterCompareFunc func, gpointer data, GDestroyNotify destroy);
static void trg_sortable_filtered_model_sort_set_default_sort_func(
        GtkTreeSortable *sortable, GtkTreeIterCompareFunc func, gpointer data,
        GDestroyNotify destroy);
static gboolean trg_sortable_filtered_model_sort_has_default_sort_func(
        GtkTreeSortable *sortable);

G_DEFINE_TYPE_WITH_CODE(
        TrgSortableFilteredModel,
        trg_sortable_filtered_model,
        GTK_TYPE_TREE_MODEL_FILTER,
        G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_SORTABLE, trg_sortable_filtered_model_tree_sortable_init))

static void trg_sortable_filtered_model_class_init(
        TrgSortableFilteredModelClass *klass) {
}

static void trg_sortable_filtered_model_init(TrgSortableFilteredModel *self) {
}

static void trg_sortable_filtered_model_tree_sortable_init(
        GtkTreeSortableIface *iface) {
    iface->get_sort_column_id =
            trg_sortable_filtered_model_sort_get_sort_column_id;
    iface->set_sort_column_id =
            trg_sortable_filtered_model_sort_set_sort_column_id;
    iface->set_sort_func = trg_sortable_filtered_model_sort_set_sort_func;
    iface->set_default_sort_func =
            trg_sortable_filtered_model_sort_set_default_sort_func;
    iface->has_default_sort_func =
            trg_sortable_filtered_model_sort_has_default_sort_func;
}

GtkTreeModel *
trg_sortable_filtered_model_new (GtkTreeSortable *child_model,
                           GtkTreePath  *root)
{
  g_return_val_if_fail (GTK_IS_TREE_MODEL (child_model), NULL);
  g_return_val_if_fail (GTK_IS_TREE_SORTABLE (child_model), NULL);

  return g_object_new (TRG_TYPE_SORTABLE_FILTERED_MODEL,
                         "child-model", GTK_TREE_MODEL(child_model),
                         "virtual-root", root,
                         NULL);
}

static gboolean trg_sortable_filtered_model_sort_get_sort_column_id(
        GtkTreeSortable *sortable, gint *sort_column_id, GtkSortType *order) {
    GtkTreeSortable *realSortable = GTK_TREE_SORTABLE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(sortable)));
    return gtk_tree_sortable_get_sort_column_id(realSortable, sort_column_id, order);
}

static void trg_sortable_filtered_model_sort_set_sort_column_id(
        GtkTreeSortable *sortable, gint sort_column_id, GtkSortType order)
{
    GtkTreeSortable *realSortable = GTK_TREE_SORTABLE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(sortable)));
    gtk_tree_sortable_set_sort_column_id(realSortable, sort_column_id, order);
}

static void trg_sortable_filtered_model_sort_set_sort_func(
        GtkTreeSortable *sortable, gint sort_column_id,
        GtkTreeIterCompareFunc func, gpointer data, GDestroyNotify destroy)
{
    GtkTreeSortable *realSortable = GTK_TREE_SORTABLE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(sortable)));
    gtk_tree_sortable_set_sort_func(realSortable, sort_column_id, func, data, destroy);
}

static void trg_sortable_filtered_model_sort_set_default_sort_func(
        GtkTreeSortable *sortable, GtkTreeIterCompareFunc func, gpointer data,
        GDestroyNotify destroy)
{
    GtkTreeSortable *realSortable = GTK_TREE_SORTABLE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(sortable)));
    gtk_tree_sortable_set_default_sort_func(realSortable, func, data, destroy);
}

static gboolean trg_sortable_filtered_model_sort_has_default_sort_func(
        GtkTreeSortable *sortable)
{
    GtkTreeSortable *realSortable = GTK_TREE_SORTABLE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(sortable)));
    return gtk_tree_sortable_has_default_sort_func(realSortable);
}
