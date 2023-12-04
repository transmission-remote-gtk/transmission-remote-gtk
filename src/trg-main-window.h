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

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "trg-files-model.h"
#include "trg-general-panel.h"
#include "trg-peers-model.h"
#include "trg-torrent-model.h"
#include "trg-torrent-tree-view.h"
#include "trg-trackers-model.h"

G_BEGIN_DECLS

#define TRG_TYPE_MAIN_WINDOW (trg_main_window_get_type())
G_DECLARE_FINAL_TYPE(TrgMainWindow, trg_main_window, TRG, MAIN_WINDOW, GtkApplicationWindow)

gint trg_add_from_filename(TrgMainWindow *win, gchar **uris);
gboolean on_session_set(gpointer data);
gboolean on_delete_complete(gpointer data);
void on_generic_interactive_action(TrgMainWindow *win, trg_response *response);
gboolean on_generic_interactive_action_response(gpointer data);
void auto_connect_if_required(TrgMainWindow *win);
void trg_main_window_set_start_args(TrgMainWindow *win, gchar **args);
TrgMainWindow *trg_main_window_new(TrgClient *tc);
void trg_main_window_add_tray(TrgMainWindow *win);
void trg_main_window_remove_tray(TrgMainWindow *win);
void trg_main_window_add_graph(TrgMainWindow *win, gboolean show);
void trg_main_window_remove_graph(TrgMainWindow *win);
TrgStateSelector *trg_main_window_get_state_selector(TrgMainWindow *win);
gint trg_mw_get_selected_torrent_id(TrgMainWindow *win);
GtkTreeModel *trg_main_window_get_torrent_model(TrgMainWindow *win);
void trg_main_window_notebook_set_visible(TrgMainWindow *win, gboolean visible);
void connect_cb(GtkWidget *w, gpointer data);
void trg_main_window_reload_dir_aliases(TrgMainWindow *win);

G_END_DECLS
