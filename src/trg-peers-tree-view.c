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

#include "config.h"

#if HAVE_GEOIP
#include <GeoIP.h>
#endif

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-peers-model.h"
#include "trg-peers-tree-view.h"
#include "trg-prefs.h"
#include "trg-tree-view.h"

G_DEFINE_TYPE(TrgPeersTreeView, trg_peers_tree_view, TRG_TYPE_TREE_VIEW)
static void trg_peers_tree_view_class_init(TrgPeersTreeViewClass *klass G_GNUC_UNUSED)
{
}

static void trg_peers_tree_view_init(TrgPeersTreeView *self)
{
}

static void trg_peers_tree_view_setup_columns(TrgPeersTreeView *self, TrgPeersModel *model)
{
    TrgTreeView *ttv = TRG_TREE_VIEW(self);
    trg_column_description *desc;

    desc = trg_tree_view_reg_column(ttv, TRG_COLTYPE_ICONTEXT, PEERSCOL_IP, _("IP"), "ip", 0);
    desc->model_column_extra = PEERSCOL_ICON;

    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, PEERSCOL_HOST, _("Host"), "host", 0);

#if HAVE_GEOIP
    if (trg_peers_model_has_country_db(model))
        trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, PEERSCOL_COUNTRY, _("Country"), "country",
                                 0);

    if (trg_peers_model_has_city_db(model))
        trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, PEERSCOL_CITY, _("City"), "city", 0);
#endif
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SPEED, PEERSCOL_DOWNSPEED, _("Down Speed"),
                             "down-speed", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SPEED, PEERSCOL_UPSPEED, _("Up Speed"), "up-speed",
                             0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_PROG, PEERSCOL_PROGRESS, _("Progress"), "progress",
                             0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, PEERSCOL_FLAGS, _("Flags"), "flags", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, PEERSCOL_CLIENT, _("Client"), "client", 0);

    gtk_tree_view_set_search_column(GTK_TREE_VIEW(self), PEERSCOL_HOST);
}

#if HAVE_GEOIP
static void trg_peers_tree_view_column_added(TrgTreeView *tv, const gchar *id)
{
    TrgPeersModel *model = TRG_PEERS_MODEL(gtk_tree_view_get_model(GTK_TREE_VIEW(tv)));
    if (!g_strcmp0(id, "city")) {
        trg_peers_model_add_city_column(model);
    } else if (!g_strcmp0(id, "country")) {
        trg_peers_model_add_country_column(model);
    }
}
#endif

TrgPeersTreeView *trg_peers_tree_view_new(TrgPrefs *prefs, TrgPeersModel *model,
                                          const gchar *configId)
{
    GObject *obj
        = g_object_new(TRG_TYPE_PEERS_TREE_VIEW, "config-id", configId, "prefs", prefs, NULL);

    trg_peers_tree_view_setup_columns(TRG_PEERS_TREE_VIEW(obj), model);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));
    trg_tree_view_restore_sort(TRG_TREE_VIEW(obj), 0x00);
    trg_tree_view_setup_columns(TRG_TREE_VIEW(obj));

#if HAVE_GEOIP
    g_signal_connect(obj, "column-added", G_CALLBACK(trg_peers_tree_view_column_added), NULL);
#endif

    return TRG_PEERS_TREE_VIEW(obj);
}
