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

#include "trg-client.h"
#include "trg-main-window.h"
#include "trg-trackers-model.h"
#include "trg-tree-view.h"

#define TRG_TYPE_TRACKERS_TREE_VIEW trg_trackers_tree_view_get_type()
G_DECLARE_FINAL_TYPE(TrgTrackersTreeView, trg_trackers_tree_view, TRG, TRACKERS_TREE_VIEW,
                     TrgTreeView)

TrgTrackersTreeView *trg_trackers_tree_view_new(TrgTrackersModel *model, TrgClient *client,
                                                TrgMainWindow *win, const gchar *configId);
void trg_trackers_tree_view_new_connection(TrgTrackersTreeView *tv, TrgClient *tc);
