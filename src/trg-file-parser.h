/*
 * transmission-remote-gtk - Transmission RPC client for GTK
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

typedef struct {
    char *name;
    gint64 length;
    GList *children;
    guint index;
} trg_torrent_file_node;

typedef struct {
    char *name;
    trg_torrent_file_node *top_node;
    gint64 total_length;
} trg_torrent_file;

void trg_torrent_file_free(trg_torrent_file * t);
trg_torrent_file *trg_parse_torrent_file(const gchar * filename);
