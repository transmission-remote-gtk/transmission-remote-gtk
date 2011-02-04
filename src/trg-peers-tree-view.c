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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_GEOIP
#include <GeoIP.h>
#endif

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-tree-view.h"
#include "trg-peers-model.h"
#include "trg-peers-tree-view.h"

G_DEFINE_TYPE(TrgPeersTreeView, trg_peers_tree_view, TRG_TYPE_TREE_VIEW)

static void
trg_peers_tree_view_class_init(TrgPeersTreeViewClass * klass G_GNUC_UNUSED)
{
}

static void trg_peers_tree_view_init(TrgPeersTreeView * self)
{
    trg_tree_view_add_pixbuf_text_column(TRG_TREE_VIEW
					 (self),
					 PEERSCOL_ICON,
					 PEERSCOL_IP, "IP", 160);
#if HAVE_GEOIP
    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Country",
			     PEERSCOL_COUNTRY);
#endif
    trg_tree_view_add_column_fixed_width(TRG_TREE_VIEW(self), "Host",
					 PEERSCOL_HOST, 250);
    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Client",
                             PEERSCOL_CLIENT);
    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Flags", PEERSCOL_FLAGS);
    trg_tree_view_add_prog_column(TRG_TREE_VIEW(self), "Progress",
				  PEERSCOL_PROGRESS, -1);
    trg_tree_view_add_speed_column(TRG_TREE_VIEW(self), "Down Speed",
				   PEERSCOL_DOWNSPEED, -1);
    trg_tree_view_add_speed_column(TRG_TREE_VIEW(self), "Up Speed",
				   PEERSCOL_UPSPEED, -1);

}

TrgPeersTreeView *trg_peers_tree_view_new(TrgPeersModel * model)
{
    GObject *obj = g_object_new(TRG_TYPE_PEERS_TREE_VIEW, NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));

    return TRG_PEERS_TREE_VIEW(obj);
}
