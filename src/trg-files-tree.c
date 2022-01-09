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

/* This is the stuff common between both files trees, built up before
 * populating the model.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "trg-files-tree.h"

void trg_files_tree_node_add_child(trg_files_tree_node* node, trg_files_tree_node* child)
{
  if (!node->childrenHash) {
    node->childrenHash = g_hash_table_new(g_str_hash, g_str_equal);
  }
  g_hash_table_insert(node->childrenHash, child->name, child);
  node->children = g_list_append(node->children, child);
}

void trg_files_tree_node_free(trg_files_tree_node * node)
{
    GList *li;

    for (li = node->children; li; li = g_list_next(li))
        trg_files_tree_node_free((trg_files_tree_node *) li->data);

    if (node->childrenHash)
      g_hash_table_destroy(node->childrenHash);
    
    g_list_free(node->children);
    g_free(node->name);
    g_free(node);
}
