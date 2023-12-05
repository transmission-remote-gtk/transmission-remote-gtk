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
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "trg-torrent-model.h"

enum {
    STATE_SELECTOR_ICON,
    STATE_SELECTOR_NAME,
    STATE_SELECTOR_COUNT,
    STATE_SELECTOR_BIT,
    STATE_SELECTOR_SERIAL,
    STATE_SELECTOR_INDEX,
    STATE_SELECTOR_COLUMNS
};

#define TRG_TYPE_STATE_SELECTOR trg_state_selector_get_type()
G_DECLARE_FINAL_TYPE(TrgStateSelector, trg_state_selector, TRG, STATE_SELECTOR, GtkTreeView)

TrgStateSelector *trg_state_selector_new(TrgClient *client, TrgTorrentModel *tmodel);

guint32 trg_state_selector_get_flag(TrgStateSelector *s);
void trg_state_selector_update(TrgStateSelector *s, guint whatsChanged);
gchar *trg_state_selector_get_selected_text(TrgStateSelector *s);
GRegex *trg_state_selector_get_url_host_regex(TrgStateSelector *s);
void trg_state_selector_disconnect(TrgStateSelector *s);
void trg_state_selector_set_show_trackers(TrgStateSelector *s, gboolean show);
void trg_state_selector_set_directories_first(TrgStateSelector *s, gboolean _dirsFirst);
void trg_state_selector_set_show_dirs(TrgStateSelector *s, gboolean show);
void trg_state_selector_set_queues_enabled(TrgStateSelector *s, gboolean enabled);
void trg_state_selector_stats_update(TrgStateSelector *s, trg_torrent_model_update_stats *stats);
