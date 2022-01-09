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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-prefs.h"
#include "trg-tree-view.h"
#include "trg-torrent-model.h"
#include "torrent-cell-renderer.h"
#include "trg-torrent-tree-view.h"

G_DEFINE_TYPE(TrgTorrentTreeView, trg_torrent_tree_view,
              TRG_TYPE_TREE_VIEW)
#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_TREE_VIEW, TrgTorrentTreeViewPrivate))
typedef struct _TrgTorrentTreeViewPrivate TrgTorrentTreeViewPrivate;

struct _TrgTorrentTreeViewPrivate {
    TrgClient *client;
};

static void trg_torrent_tree_view_class_init(TrgTorrentTreeViewClass *
                                             klass G_GNUC_UNUSED)
{
    g_type_class_add_private(klass, sizeof(TrgTorrentTreeViewPrivate));
}

static void trg_torrent_tree_view_init(TrgTorrentTreeView * tttv)
{
    TrgTreeView *ttv = TRG_TREE_VIEW(tttv);
    trg_column_description *desc;

    desc =
        trg_tree_view_reg_column(ttv, TRG_COLTYPE_ICONTEXT,
                                 TORRENT_COLUMN_NAME, _("Name"), "name",
                                 0);
    desc->model_column_extra = TORRENT_COLUMN_ICON;

    trg_tree_view_reg_column(ttv, TRG_COLTYPE_SIZE,
                             TORRENT_COLUMN_SIZEWHENDONE, _("Size"),
                             "size", 0);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_PROG,
                             TORRENT_COLUMN_PERCENTDONE, _("Done"), "done",
                             0);
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
                             "from-pex",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMDHT, _("DHT Peers"),
                             "from-dht",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMTRACKERS,
                             _("Tracker Peers"), "from-trackers",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMLTEP, _("LTEP Peers"),
                             "from-ltep",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMRESUME, _("Resumed Peers"),
                             "from-resume",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_NUMGTZERO,
                             TORRENT_COLUMN_FROMINCOMING,
                             _("Incoming Peers"), "from-incoming",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
    trg_tree_view_reg_column(ttv, TRG_COLTYPE_TEXT,
                             TORRENT_COLUMN_PEER_SOURCES,
                             _("Peers T/I/E/H/X/L/R"), "peer-sources",
                             TRG_COLUMN_EXTRA |
                             TRG_COLUMN_HIDE_FROM_TOP_MENU);
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

static void setup_classic_layout(TrgTorrentTreeView * tv)
{
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(tv), TRUE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(tv), TRUE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv), TRUE);
    trg_tree_view_setup_columns(TRG_TREE_VIEW(tv));
}

static void
trg_torrent_tree_view_renderer_pref_changed(TrgPrefs * p,
                                            const gchar * updatedKey,
                                            gpointer data)
{
    if (!g_strcmp0(updatedKey, TRG_PREFS_KEY_STYLE)) {
        GtkTreeView *tv =
            torrent_cell_renderer_get_owner(TORRENT_CELL_RENDERER(data));
        gboolean compact = trg_prefs_get_int(p, TRG_PREFS_KEY_STYLE,
                                             TRG_PREFS_GLOBAL) ==
            TRG_STYLE_TR_COMPACT;
        g_object_set(G_OBJECT(data), "compact", GINT_TO_POINTER(compact),
                     NULL);
        g_signal_emit_by_name(tv, "style-updated", NULL, NULL);
    }
}

static void setup_transmission_layout(TrgTorrentTreeView * tv,
                                      gint64 style)
{
    TrgTorrentTreeViewPrivate *priv = GET_PRIVATE(tv);
    GtkCellRenderer *renderer = torrent_cell_renderer_new();
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    GtkTreeViewColumn *column =
        gtk_tree_view_column_new_with_attributes("",
                                                 renderer,
                                                 "status",
                                                 TORRENT_COLUMN_FLAGS,
                                                 "error",
                                                 TORRENT_COLUMN_ERROR,
                                                 "fileCount",
                                                 TORRENT_COLUMN_FILECOUNT,
                                                 "totalSize",
                                                 TORRENT_COLUMN_TOTALSIZE,
                                                 "ratio",
                                                 TORRENT_COLUMN_RATIO,
                                                 "downloaded",
                                                 TORRENT_COLUMN_DOWNLOADED,
                                                 "haveValid",
                                                 TORRENT_COLUMN_HAVE_VALID,
                                                 "sizeWhenDone",
                                                 TORRENT_COLUMN_SIZEWHENDONE,
                                                 "uploaded",
                                                 TORRENT_COLUMN_UPLOADED,
                                                 "percentComplete",
                                                 TORRENT_COLUMN_PERCENTDONE,
                                                 "metadataPercentComplete",
                                                 TORRENT_COLUMN_METADATAPERCENTCOMPLETE,
                                                 "upSpeed",
                                                 TORRENT_COLUMN_UPSPEED,
                                                 "downSpeed",
                                                 TORRENT_COLUMN_DOWNSPEED,
                                                 "peersToUs",
                                                 TORRENT_COLUMN_PEERS_TO_US,
                                                 "peersGettingFromUs",
                                                 TORRENT_COLUMN_PEERS_FROM_US,
                                                 "webSeedsToUs",
                                                 TORRENT_COLUMN_WEB_SEEDS_TO_US,
                                                 "eta", TORRENT_COLUMN_ETA,
                                                 "json",
                                                 TORRENT_COLUMN_JSON,
                                                 "seedRatioMode",
                                                 TORRENT_COLUMN_SEED_RATIO_MODE,
                                                 "seedRatioLimit",
                                                 TORRENT_COLUMN_SEED_RATIO_LIMIT,
                                                 "connected",
                                                 TORRENT_COLUMN_PEERS_CONNECTED,
                                                 NULL);

    g_object_set(G_OBJECT(renderer), "client", priv->client,
                 "owner", tv,
                 "compact", style == TRG_STYLE_TR_COMPACT, NULL);

    g_signal_connect_object(prefs, "pref-changed",
                            G_CALLBACK
                            (trg_torrent_tree_view_renderer_pref_changed),
                            renderer, G_CONNECT_AFTER);

    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_reorderable(column, FALSE);
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(tv), FALSE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(tv), FALSE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv), FALSE);

    gtk_tree_view_column_set_sort_column_id(column, TORRENT_COLUMN_NAME);

    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

static void
trg_torrent_tree_view_pref_changed(TrgPrefs * p, const gchar * updatedKey,
                                   gpointer data)
{
    if (!g_strcmp0(updatedKey, TRG_PREFS_KEY_STYLE)) {
        TrgTorrentTreeViewPrivate *priv = GET_PRIVATE(data);
        TrgPrefs *prefs = trg_client_get_prefs(priv->client);

        trg_tree_view_remove_all_columns(TRG_TREE_VIEW(data));
        if (trg_prefs_get_int(p, TRG_PREFS_KEY_STYLE, TRG_PREFS_GLOBAL) ==
            TRG_STYLE_CLASSIC)
            setup_classic_layout(TRG_TORRENT_TREE_VIEW(data));
        else
            setup_transmission_layout(TRG_TORRENT_TREE_VIEW(data),
                                      trg_prefs_get_int(prefs,
                                                        TRG_PREFS_KEY_STYLE,
                                                        TRG_PREFS_GLOBAL));
    }
}

TrgTorrentTreeView *trg_torrent_tree_view_new(TrgClient * tc,
                                              GtkTreeModel * model)
{
    GObject *obj = g_object_new(TRG_TYPE_TORRENT_TREE_VIEW, NULL);
    TrgTorrentTreeViewPrivate *priv = GET_PRIVATE(obj);
    TrgPrefs *prefs = trg_client_get_prefs(tc);
    gint64 style =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_STYLE, TRG_PREFS_GLOBAL);

    trg_tree_view_set_prefs(TRG_TREE_VIEW(obj), trg_client_get_prefs(tc));
    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), model);

    priv->client = tc;

    if (style == TRG_STYLE_CLASSIC) {
        setup_classic_layout(TRG_TORRENT_TREE_VIEW(obj));
    } else {
        setup_transmission_layout(TRG_TORRENT_TREE_VIEW(obj), style);
    }

    g_signal_connect(prefs, "pref-changed",
                     G_CALLBACK(trg_torrent_tree_view_pref_changed), obj);

    trg_tree_view_restore_sort(TRG_TREE_VIEW(obj),
                               TRG_TREE_VIEW_SORTABLE_PARENT);

    return TRG_TORRENT_TREE_VIEW(obj);
}
