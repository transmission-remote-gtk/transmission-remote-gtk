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


#ifndef TRG_TORRENT_MODEL_H_
#define TRG_TORRENT_MODEL_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"

G_BEGIN_DECLS
#define TRG_TYPE_TORRENT_MODEL trg_torrent_model_get_type()
#define TRG_TORRENT_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_TORRENT_MODEL, TrgTorrentModel))
#define TRG_TORRENT_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_TORRENT_MODEL, TrgTorrentModelClass))
#define TRG_IS_TORRENT_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_TORRENT_MODEL))
#define TRG_IS_TORRENT_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_TORRENT_MODEL))
#define TRG_TORRENT_MODEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_TORRENT_MODEL, TrgTorrentModelClass))
    typedef struct {
    GtkListStore parent;
} TrgTorrentModel;

typedef struct {
    GtkListStoreClass parent_class;
    void (*torrent_completed) (TrgTorrentModel * model,
                               GtkTreeIter * iter, gpointer data);
    void (*torrent_added) (TrgTorrentModel * model,
                           GtkTreeIter * iter, gpointer data);

    void (*torrent_removed) (TrgTorrentModel * model, gpointer data);
} TrgTorrentModelClass;

typedef struct {
    gint64 downRateTotal;
    gint64 upRateTotal;
    gint seeding;
    gint down;
    gint paused;
    gint count;
} trg_torrent_model_update_stats;

GType trg_torrent_model_get_type(void);

TrgTorrentModel *trg_torrent_model_new();

G_END_DECLS
    gboolean
find_existing_peer_item(GtkListStore * model, JsonObject * p,
                        GtkTreeIter * iter);

void trg_torrent_model_update(TrgTorrentModel * model, trg_client * tc,
                              JsonObject * response,
                              trg_torrent_model_update_stats * stats,
                              gint mode);

GHashTable *get_torrent_table(TrgTorrentModel * model);

enum {
    TORRENT_COLUMN_ICON,
    TORRENT_COLUMN_NAME,
    TORRENT_COLUMN_SIZE,
    TORRENT_COLUMN_DONE,
    TORRENT_COLUMN_STATUS,
    TORRENT_COLUMN_SEEDS,
    TORRENT_COLUMN_LEECHERS,
    TORRENT_COLUMN_DOWNSPEED,
    TORRENT_COLUMN_UPSPEED,
    TORRENT_COLUMN_ETA,
    TORRENT_COLUMN_UPLOADED,
    TORRENT_COLUMN_DOWNLOADED,
    TORRENT_COLUMN_RATIO,
    TORRENT_COLUMN_ADDED,
    TORRENT_COLUMN_ID,
    TORRENT_COLUMN_JSON,
    TORRENT_COLUMN_UPDATESERIAL,
    TORRENT_COLUMN_FLAGS,
    TORRENT_COLUMN_COLUMNS
};

#endif                          /* TRG_TORRENT_MODEL_H_ */
