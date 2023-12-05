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
#include "trg-torrent-model.h"

#define TRG_TYPE_STATUS_BAR trg_status_bar_get_type()
G_DECLARE_FINAL_TYPE(TrgStatusBar, trg_status_bar, TRG, STATUS_BAR, GtkBox)

TrgStatusBar *trg_status_bar_new(TrgMainWindow *win, TrgClient *client);
void trg_status_bar_update(TrgStatusBar *sb, trg_torrent_model_update_stats *stats,
                           TrgClient *client);
void trg_status_bar_session_update(TrgStatusBar *sb, JsonObject *session);
void trg_status_bar_connect(TrgStatusBar *sb, JsonObject *session, TrgClient *client);
void trg_status_bar_push_connection_msg(TrgStatusBar *sb, const gchar *msg);
void trg_status_bar_reset(TrgStatusBar *sb);
void trg_status_bar_clear_indicators(TrgStatusBar *sb);
const gchar *trg_status_bar_get_speed_text(TrgStatusBar *s);
void trg_status_bar_update_speed(TrgStatusBar *sb, trg_torrent_model_update_stats *stats,
                                 TrgClient *client);
