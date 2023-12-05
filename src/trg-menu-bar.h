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

#include "trg-main-window.h"
#include "trg-prefs.h"
#include "trg-torrent-tree-view.h"

#define TRG_TYPE_MENU_BAR trg_menu_bar_get_type()
G_DECLARE_FINAL_TYPE(TrgMenuBar, trg_menu_bar, TRG, MENU_BAR, GtkMenuBar)

TrgMenuBar *trg_menu_bar_new(TrgMainWindow *win, TrgPrefs *prefs, TrgTorrentTreeView *ttv,
                             GtkAccelGroup *accel_group);
GtkWidget *trg_menu_bar_item_new(GtkMenuShell *shell, const gchar *text, gboolean sensitive);

void trg_menu_bar_torrent_actions_sensitive(TrgMenuBar *mb, gboolean sensitive);
void trg_menu_bar_connected_change(TrgMenuBar *mb, gboolean connected);
void trg_menu_bar_set_supports_queues(TrgMenuBar *mb, gboolean supportsQueues);
GtkWidget *trg_menu_bar_file_connect_menu_new(TrgMainWindow *win, TrgPrefs *p);
