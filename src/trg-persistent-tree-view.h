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
#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-preferences-dialog.h"

#define TRG_TYPE_PERSISTENT_TREE_VIEW trg_persistent_tree_view_get_type()
G_DECLARE_FINAL_TYPE(TrgPersistentTreeView, trg_persistent_tree_view, TRG, PERSISTENT_TREE_VIEW,
                     GtkBox)

typedef struct {
    GtkTreeViewColumn *column;
    gchar *key;
    gchar *label;
    TrgPersistentTreeView *tv;
    gint index;
} trg_persistent_tree_view_column;

TrgPersistentTreeView *trg_persistent_tree_view_new(TrgPrefs *prefs, GtkListStore *model,
                                                    const gchar *key, gint conf_flags);

trg_pref_widget_desc *trg_persistent_tree_view_get_widget_desc(TrgPersistentTreeView *ptv);

void trg_persistent_tree_view_set_add_select(TrgPersistentTreeView *ptv,
                                             trg_persistent_tree_view_column *cd);

trg_persistent_tree_view_column *trg_persistent_tree_view_add_column(TrgPersistentTreeView *ptv,
                                                                     gint index, const gchar *key,
                                                                     const gchar *label);
