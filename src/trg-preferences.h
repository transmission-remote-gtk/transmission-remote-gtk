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

#ifndef TRG_PREFERENCES_H_
#define TRG_PREFERENCES_H_

#define TRG_GCONF_KEY_HOSTNAME      "/apps/transmission-remote-gtk/hostname"
#define TRG_GCONF_KEY_PORT          "/apps/transmission-remote-gtk/port"
#define TRG_GCONF_KEY_USERNAME      "/apps/transmission-remote-gtk/username"
#define TRG_GCONF_KEY_PASSWORD      "/apps/transmission-remote-gtk/password"
#define TRG_GCONF_KEY_AUTO_CONNECT  "/apps/transmission-remote-gtk/auto-connect"
#define TRG_GCONF_KEY_SSL			"/apps/transmission-remote-gtk/ssl"
#define TRG_GCONF_KEY_UPDATE_INTERVAL "/apps/transmission-remote-gtk/update-interval"
#define TRG_GCONF_KEY_COMPLETE_NOTIFY "/apps/transmission-remote-gtk/complete-notify"
#define TRG_GCONF_KEY_ADD_NOTIFY    "/apps/transmission-remote-gtk/add-notify"
#define TRG_GCONF_KEY_WINDOW_WIDTH  "/apps/transmission-remote-gtk/window-width"
#define TRG_GCONF_KEY_WINDOW_HEIGHT "/apps/transmission-remote-gtk/window-height"
#define TRG_GCONF_KEY_GRAPH_SPAN  "/apps/transmission-remote-gtk/graph-span"
#define TRG_GCONF_KEY_SYSTEM_TRAY   "/apps/transmission-remote-gtk/system-tray"
#define TRG_GCONF_KEY_SHOW_GRAPH    "/apps/transmission-remote-gtk/show-graph"
#define TRG_GCONF_KEY_SYSTEM_TRAY_MINIMISE  "/apps/transmission-remote-gtk/system-tray-minimise"
#define TRG_GCONF_KEY_FILTER_TRACKERS  "/apps/transmission-remote-gtk/filter-trackers"
#define TRG_GCONF_KEY_FILTER_DIRS  "/apps/transmission-remote-gtk/filter-dirs"
#define TRG_GCONF_KEY_LAST_TORRENT_DIR "/apps/transmission-remote-gtk/last-torrent-dir"
#define TRG_GCONF_KEY_ADD_OPTIONS_DIALOG "/apps/transmission-remote-gtk/add-options-dialog"
#define TRG_GCONF_KEY_START_PAUSED "/apps/transmission-remote-gtk/start-paused"

gboolean pref_get_start_paused(GConfClient *gcc);

#endif                          /* TRG_PREFERENCES_H_ */
