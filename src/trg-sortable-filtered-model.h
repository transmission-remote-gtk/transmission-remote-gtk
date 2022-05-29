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

#ifndef _TRG_SORTABLE_FILTERED_MODEL
#define _TRG_SORTABLE_FILTERED_MODEL

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TRG_TYPE_SORTABLE_FILTERED_MODEL trg_sortable_filtered_model_get_type()
#define TRG_SORTABLE_FILTERED_MODEL(obj)                                                           \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_SORTABLE_FILTERED_MODEL, TrgSortableFilteredModel))
#define TRG_SORTABLE_FILTERED_MODEL_CLASS(klass)                                                   \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_SORTABLE_FILTERED_MODEL,                            \
                             TrgSortableFilteredModelClass))
#define TRG_IS_SORTABLE_FILTERED_MODEL(obj)                                                        \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_SORTABLE_FILTERED_MODEL))
#define TRG_IS_SORTABLE_FILTERED_MODEL_CLASS(klass)                                                \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_SORTABLE_FILTERED_MODEL))
#define TRG_SORTABLE_FILTERED_MODEL_GET_CLASS(obj)                                                 \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_SORTABLE_FILTERED_MODEL,                            \
                               TrgSortableFilteredModelClass))
typedef struct {
    GtkTreeModelFilter parent;
} TrgSortableFilteredModel;

typedef struct {
    GtkTreeModelFilterClass parent_class;
} TrgSortableFilteredModelClass;

GType trg_sortable_filtered_model_get_type(void);

GtkTreeModel *trg_sortable_filtered_model_new(GtkTreeSortable *child_model, GtkTreePath *root);

G_END_DECLS
#endif /* _TRG_SORTABLE_FILTERED_MODEL */
