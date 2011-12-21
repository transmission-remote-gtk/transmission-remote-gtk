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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-prefs.h"
#include "trg-tree-view.h"
#include "trg-torrent-model.h"
#include "trg-torrent-tree-view.h"

G_DEFINE_TYPE(TrgTorrentTreeView, trg_torrent_tree_view,
              TRG_TYPE_TREE_VIEW)

static void
trg_torrent_tree_view_class_init(TrgTorrentTreeViewClass *
                                 klass G_GNUC_UNUSED)
{
}

static void trg_torrent_tree_view_init(TrgTorrentTreeView * tttv)
{
    TrgTreeView *ttv = TRG_TREE_VIEW(tttv);
    trg_column_description *desc;

    desc =
        trg_tree_view_reg_column(ttv, TRG_COLTYPE_STOCKICONTEXT,
                                 TORRENT_COLUMN_NAME, _("Name"), "name",
                                 0);
    desc->model_column_extra = TORRENT_COLUMN_ICON;

    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SIZE, TORRENT_COLUMN_SIZE,
                             _("Size"), "size", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_PROG, TORRENT_COLUMN_DONE,
                             _("Done"), "done", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, TORRENT_COLUMN_STATUS,
                             _("Status"), "status", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_SEEDS, _("Seeds"), "seeds", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_PEERS_TO_US, _("Sending"),
                             "sending", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_LEECHERS, _("Leechers"),
                             "leechers", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_DOWNLOADS, _("Downloads"),
                             "downloads", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_PEERS_FROM_US, _("Receiving"),
                             "connected-leechers", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_PEERS_CONNECTED,
                             _("Connected"), "connected-peers", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMPEX, _("PEX Peers"),
                             "from-pex", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMDHT, _("DHT Peers"),
                             "from-dht", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMTRACKERS,
                             _("Tracker Peers"), "from-trackers",
                             TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMLTEP, _("LTEP Peers"),
                             "from-ltep", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMRESUME, _("Resumed Peers"),
                             "from-resume", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMINCOMING,
                             _("Incoming Peers"), "from-incoming",
                             TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT,
                             TORRENT_COLUMN_PEER_SOURCES,
                             _("Peers T/I/E/H/X/L/R"), "peer-sources",
                             TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SPEED,
                             TORRENT_COLUMN_DOWNSPEED, _("Down Speed"),
                             "down-speed", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SPEED,
                             TORRENT_COLUMN_UPSPEED, _("Up Speed"),
                             "up-speed", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_ETA, TORRENT_COLUMN_ETA,
                             _("ETA"), "eta", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SIZE,
                             TORRENT_COLUMN_UPLOADED, _("Uploaded"),
                             "uploaded", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SIZE,
                             TORRENT_COLUMN_DOWNLOADED, _("Downloaded"),
                             "downloaded", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_RATIO, TORRENT_COLUMN_RATIO,
                             _("Ratio"), "ratio", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_EPOCH, TORRENT_COLUMN_ADDED,
                             _("Added"), "added", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT,
                             TORRENT_COLUMN_TRACKERHOST,
                             _("First Tracker"), "first-tracker",
                             TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT,
                             TORRENT_COLUMN_DOWNLOADDIR, _("Location"),
                             "download-dir", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT, TORRENT_COLUMN_ID,
                             _("ID"), "id", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_PRIO,
                             TORRENT_COLUMN_BANDWIDTH_PRIORITY,
                             _("Priority"), "priority", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTEQZERO,
                             TORRENT_COLUMN_QUEUE_POSITION,
                             _("Queue Position"), "queue-position",
                             TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_EPOCH,
                             TORRENT_COLUMN_DONE_DATE, _("Completed"),
                             "done-date", TRG_COLUMN_EXTRA);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_EPOCH,
                             TORRENT_COLUMN_LASTACTIVE, _("Last Active"),
                             "last-active", TRG_COLUMN_EXTRA);


    gtk_tree_view_set_search_column(GTK_TREE_VIEW(tttv),
                                    TORRENT_COLUMN_NAME);
}

static void
trg_torrent_model_get_json_id_array_foreach(GtkTreeModel * model,
                                            GtkTreePath *
                                            path G_GNUC_UNUSED,
                                            GtkTreeIter * iter,
                                            gpointer data)
{
    JsonArray *output = (JsonArray *) data;
    gint64 id;
    gtk_tree_model_get(model, iter, TORRENT_COLUMN_ID, &id, -1);
    json_array_add_int_element(output, id);
}

JsonArray *build_json_id_array(TrgTorrentTreeView * tv)
{
    GtkTreeSelection *selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(tv));

    JsonArray *ids = json_array_new();
    gtk_tree_selection_selected_foreach(selection,
                                        (GtkTreeSelectionForeachFunc)
                                        trg_torrent_model_get_json_id_array_foreach,
                                        ids);

    return ids;
}

TrgTorrentTreeView *trg_torrent_tree_view_new(TrgPrefs * prefs,
                                              GtkTreeModel * model)
{
    GObject *obj = g_object_new(TRG_TYPE_TORRENT_TREE_VIEW, NULL);

    trg_tree_view_set_prefs(TRG_TREE_VIEW(obj), prefs);
    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), model);
    trg_tree_view_setup_columns(TRG_TREE_VIEW(obj));

    trg_tree_view_restore_sort(TRG_TREE_VIEW(obj), TRUE);

    return TRG_TORRENT_TREE_VIEW(obj);
}
