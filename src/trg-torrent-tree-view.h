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

#ifndef _TRG_TORRENT_TREE_VIEW_H_
#define _TRG_TORRENT_TREE_VIEW_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-prefs.h"
#include "trg-torrent-model.h"
#include "trg-tree-view.h"
#include "trg-state-selector.h"

G_BEGIN_DECLS
#define TRG_TYPE_TORRENT_TREE_VIEW trg_torrent_tree_view_get_type()
#define TRG_TORRENT_TREE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_TORRENT_TREE_VIEW, TrgTorrentTreeView))
#define TRG_TORRENT_TREE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_TORRENT_TREE_VIEW, TrgTorrentTreeViewClass))
#define TRG_IS_TORRENT_TREE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_TORRENT_TREE_VIEW))
#define TRG_IS_TORRENT_TREE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_TORRENT_TREE_VIEW))
#define TRG_TORRENT_TREE_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_TORRENT_TREE_VIEW, TrgTorrentTreeViewClass))
    typedef struct {
    TrgTreeView parent;
} TrgTorrentTreeView;

typedef struct {
    TrgTreeViewClass parent_class;
} TrgTorrentTreeViewClass;

GType trg_torrent_tree_view_get_type(void);

TrgTorrentTreeView *trg_torrent_tree_view_new(TrgClient * tc,
                                              GtkTreeModel * model);
JsonArray *build_json_id_array(TrgTorrentTreeView * tv);

G_END_DECLS
#endif                          /* _TRG_TORRENT_TREE_VIEW_H_ */
