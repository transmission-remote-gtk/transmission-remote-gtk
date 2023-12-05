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

typedef enum {
    /* we won't (announce,scrape) this torrent to this tracker because
     * the torrent is stopped, or because of an error, or whatever */
    TR_TRACKER_INACTIVE = 0,

    /* we will (announce,scrape) this torrent to this tracker, and are
     * waiting for enough time to pass to satisfy the tracker's interval */
    TR_TRACKER_WAITING = 1,

    /* it's time to (announce,scrape) this torrent, and we're waiting on a
     * a free slot to open up in the announce manager */
    TR_TRACKER_QUEUED = 2,

    /* we're (announcing,scraping) this torrent right now */
    TR_TRACKER_ACTIVE = 3
} tr_tracker_state;

enum {
    /* trackers */
    TRACKERCOL_ICON,
    TRACKERCOL_TIER,
    TRACKERCOL_ANNOUNCE,
    TRACKERCOL_SCRAPE,
    TRACKERCOL_ID,
    /* trackerstats */
    TRACKERCOL_LAST_ANNOUNCE_PEER_COUNT,
    TRACKERCOL_LAST_ANNOUNCE_TIME,
    TRACKERCOL_LAST_SCRAPE_TIME,
    TRACKERCOL_SEEDERCOUNT,
    TRACKERCOL_LEECHERCOUNT,
    TRACKERCOL_HOST,
    TRACKERCOL_LAST_ANNOUNCE_RESULT,
    /* other */
    TRACKERCOL_UPDATESERIAL,
    TRACKERCOL_COLUMNS
};

#define TRG_TYPE_TRACKERS_MODEL trg_trackers_model_get_type()
G_DECLARE_FINAL_TYPE(TrgTrackersModel, trg_trackers_model, TRG, TRACKERS_MODEL, GtkListStore)

TrgTrackersModel *trg_trackers_model_new(void);
void trg_trackers_model_update(TrgTrackersModel *model, gint64 updateSerial, JsonObject *t,
                               gint mode);
void trg_trackers_model_set_accept(TrgTrackersModel *model, gboolean accept);
gint64 trg_trackers_model_get_torrent_id(TrgTrackersModel *model);
void trg_trackers_model_set_no_selection(TrgTrackersModel *model);
