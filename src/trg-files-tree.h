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

#ifndef TRG_FILES_TREE_H_
#define TRG_FILES_TREE_H_

#include <glib.h>
#include <json-glib/json-glib.h>

typedef struct {
    gchar *name;
    gint64 length;
    gint64 bytesCompleted;
    GList *children;
    GHashTable *childrenHash;
    gint index;
    gpointer parent;
    gint priority;
    gint enabled;
} trg_files_tree_node;

void trg_files_tree_node_add_child(trg_files_tree_node *node, trg_files_tree_node *child);
void trg_files_tree_node_free(trg_files_tree_node *node);

#endif /* TRG_FILES_MODEL_H_ */
