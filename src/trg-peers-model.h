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


#ifndef TRG_PEERS_MODEL_H_
#define TRG_PEERS_MODEL_H_

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#ifdef HAVE_GEOIP
#include <GeoIP.h>
#endif
#include <glib-object.h>

#include "trg-tree-view.h"

G_BEGIN_DECLS
#define TRG_TYPE_PEERS_MODEL trg_peers_model_get_type()
#define TRG_PEERS_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_PEERS_MODEL, TrgPeersModel))
#define TRG_PEERS_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_PEERS_MODEL, TrgPeersModelClass))
#define TRG_IS_PEERS_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_PEERS_MODEL))
#define TRG_IS_PEERS_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_PEERS_MODEL))
#define TRG_PEERS_MODEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_PEERS_MODEL, TrgPeersModelClass))
    typedef struct {
    GtkListStore parent;
} TrgPeersModel;

typedef struct {
    GtkListStoreClass parent_class;
} TrgPeersModelClass;

GType trg_peers_model_get_type(void);

TrgPeersModel *trg_peers_model_new();

G_END_DECLS struct peerAndIter {
    const gchar *ip;
    GtkTreeIter iter;
    gboolean found;
};

enum {
    PEERSCOL_ICON,
    PEERSCOL_IP,
#if HAVE_GEOIP
    PEERSCOL_COUNTRY,
#endif
    PEERSCOL_HOST,
    PEERSCOL_FLAGS,
    PEERSCOL_PROGRESS,
    PEERSCOL_DOWNSPEED,
    PEERSCOL_UPSPEED,
    PEERSCOL_CLIENT,
    PEERSCOL_UPDATESERIAL,
    PEERSCOL_COLUMNS
};

void trg_peers_model_update(TrgPeersModel * model, TrgTreeView * tv,
                            gint64 updateSerial, JsonObject * t,
                            gboolean first);

#endif                          /* TRG_PEERS_MODEL_H_ */

#define TRG_GEOIP_DATABASE "/usr/share/GeoIP/GeoIP.dat"
#define TRG_GEOIPV6_DATABASE "/usr/share/GeoIP/GeoIPv6.dat"
