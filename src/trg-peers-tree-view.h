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


#ifndef TRG_PEERS_TREE_VIEW_H_
#define TRG_PEERS_TREE_VIEW_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-prefs.h"
#include "trg-peers-model.h"

G_BEGIN_DECLS
#define TRG_TYPE_PEERS_TREE_VIEW trg_peers_tree_view_get_type()
#define TRG_PEERS_TREE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_PEERS_TREE_VIEW, TrgPeersTreeView))
#define TRG_PEERS_TREE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_PEERS_TREE_VIEW, TrgPeersTreeViewClass))
#define TRG_IS_PEERS_TREE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_PEERS_TREE_VIEW))
#define TRG_IS_PEERS_TREE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_PEERS_TREE_VIEW))
#define TRG_PEERS_TREE_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_PEERS_TREE_VIEW, TrgPeersTreeViewClass))
    typedef struct {
    GtkTreeView parent;
} TrgPeersTreeView;

typedef struct {
    GtkTreeViewClass parent_class;
} TrgPeersTreeViewClass;

GType trg_peers_tree_view_get_type(void);

TrgPeersTreeView *trg_peers_tree_view_new(TrgPrefs * prefs,
                                          TrgPeersModel * model,
                                          const gchar * configId);

G_END_DECLS
#endif                          /* TRG_PEERS_TREE_VIEW_H_ */
