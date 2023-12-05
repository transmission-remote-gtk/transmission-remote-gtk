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
#include <json-glib/json-glib.h>

#include "trg-prefs.h"
#include "trg-state-selector.h"
#include "trg-torrent-model.h"
#include "trg-tree-view.h"

G_BEGIN_DECLS
#define TRG_TYPE_TORRENT_TREE_VIEW trg_torrent_tree_view_get_type()
G_DECLARE_FINAL_TYPE(TrgTorrentTreeView, trg_torrent_tree_view, TRG, TORRENT_TREE_VIEW, TrgTreeView)

TrgTorrentTreeView *trg_torrent_tree_view_new(TrgClient *tc, GtkTreeModel *model);
JsonArray *build_json_id_array(TrgTorrentTreeView *tv);
