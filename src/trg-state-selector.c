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

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "torrent.h"
#include "trg-cell-renderer-counter.h"
#include "trg-client.h"
#include "trg-prefs.h"
#include "trg-state-selector.h"
#include "trg-torrent-model.h"
#include "util.h"

enum {
    SELECTOR_STATE_CHANGED,
    SELECTOR_SIGNAL_COUNT
};

enum {
    PROP_0,
    PROP_CLIENT
};

static guint signals[SELECTOR_SIGNAL_COUNT] = { 0 };
struct _TrgStateSelector {
    GtkTreeView parent;

    guint flag;
    gboolean showDirs;
    gboolean showTrackers;
    gboolean dirsFirst;
    TrgClient *client;
    TrgPrefs *prefs;
    GHashTable *trackers;
    GHashTable *directories;
    GRegex *urlHostRegex;
    gint n_categories;
    GtkListStore *store;
    GtkTreeRowReference *error_rr;
    GtkTreeRowReference *all_rr;
    GtkTreeRowReference *paused_rr;
    GtkTreeRowReference *down_rr;
    GtkTreeRowReference *seeding_rr;
    GtkTreeRowReference *complete_rr;
    GtkTreeRowReference *incomplete_rr;
    GtkTreeRowReference *checking_rr;
    GtkTreeRowReference *active_rr;
    GtkTreeRowReference *seed_wait_rr;
    GtkTreeRowReference *down_wait_rr;
};

G_DEFINE_TYPE(TrgStateSelector, trg_state_selector, GTK_TYPE_TREE_VIEW)
#define TRG_STATE_SELECTOR_GET_PRIVATE(o)

GRegex *trg_state_selector_get_url_host_regex(TrgStateSelector *s)
{
    return s->urlHostRegex;
}

guint32 trg_state_selector_get_flag(TrgStateSelector *s)
{
    return s->flag;
}

static void state_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *stateModel;
    guint index = 0;

    TrgStateSelector *self = TRG_STATE_SELECTOR(data);

    if (gtk_tree_selection_get_selected(selection, &stateModel, &iter))
        gtk_tree_model_get(stateModel, &iter, STATE_SELECTOR_BIT, &self->flag, STATE_SELECTOR_INDEX,
                           &index, -1);
    else
        self->flag = 0;

    trg_prefs_set_int(self->prefs, TRG_PREFS_STATE_SELECTOR_LAST, index, TRG_PREFS_GLOBAL);

    g_signal_emit(TRG_STATE_SELECTOR(data), signals[SELECTOR_STATE_CHANGED], 0, self->flag);
}

static GtkTreeRowReference *quick_tree_ref_new(GtkTreeModel *model, GtkTreeIter *iter)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    GtkTreeRowReference *rr = gtk_tree_row_reference_new(model, path);
    gtk_tree_path_free(path);
    return rr;
}

struct cruft_remove_args {
    GHashTable *table;
    gint64 serial;
};

static gboolean trg_state_selector_remove_cruft(gpointer key, gpointer value, gpointer data)
{
    struct cruft_remove_args *args = (struct cruft_remove_args *)data;
    GtkTreeRowReference *rr = (GtkTreeRowReference *)value;
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    gboolean remove;

    GtkTreeIter iter;
    gint64 currentSerial;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, STATE_SELECTOR_SERIAL, &currentSerial, -1);

    remove = (args->serial != currentSerial);

    gtk_tree_path_free(path);

    return remove;
}

gchar *trg_state_selector_get_selected_text(TrgStateSelector *s)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(s));
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name = NULL;

    if (gtk_tree_selection_get_selected(sel, &model, &iter))
        gtk_tree_model_get(model, &iter, STATE_SELECTOR_NAME, &name, -1);

    return name;
}

static void trg_state_selector_update_dynamic_filter(GtkTreeModel *model, GtkTreeRowReference *rr,
                                                     gint64 serial)
{
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    gint64 oldSerial;
    GValue gvalue = G_VALUE_INIT;
    gint oldCount;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, STATE_SELECTOR_SERIAL, &oldSerial, STATE_SELECTOR_COUNT,
                       &oldCount, -1);

    if (oldSerial != serial) {
        g_value_init(&gvalue, G_TYPE_INT);
        g_value_set_int(&gvalue, 1);
        gtk_list_store_set_value(GTK_LIST_STORE(model), &iter, STATE_SELECTOR_COUNT, &gvalue);

        memset(&gvalue, 0, sizeof(GValue));
        g_value_init(&gvalue, G_TYPE_INT64);
        g_value_set_int64(&gvalue, serial);
        gtk_list_store_set_value(GTK_LIST_STORE(model), &iter, STATE_SELECTOR_SERIAL, &gvalue);
    } else {
        g_value_init(&gvalue, G_TYPE_INT);
        g_value_set_int(&gvalue, ++oldCount);
        gtk_list_store_set_value(GTK_LIST_STORE(model), &iter, STATE_SELECTOR_COUNT, &gvalue);
    }

    gtk_tree_path_free(path);
}

static void refresh_statelist_cb(GtkWidget *w, gpointer data)
{
    TrgStateSelector *self = TRG_STATE_SELECTOR(data);
    trg_client_inc_serial(self->client);
    trg_state_selector_update(data, TORRENT_UPDATE_ADDREMOVE);
}

static void view_popup_menu(GtkWidget *treeview, GdkEventButton *event, gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *item, *box, *img, *label;

    menu = gtk_menu_new();
    gtk_menu_set_reserve_toggle_size(GTK_MENU(menu), FALSE);

    item = gtk_menu_item_new();
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    img = gtk_image_new_from_icon_name("view-refresh", GTK_ICON_SIZE_MENU);
    label = gtk_label_new(_("Refresh"));

    gtk_container_add(GTK_CONTAINER(box), img);
    gtk_container_add(GTK_CONTAINER(box), label);

    gtk_container_add(GTK_CONTAINER(item), box);

    g_signal_connect(item, "activate", G_CALLBACK(refresh_statelist_cb), treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_widget_show_all(menu);

    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
}

static gboolean view_onPopupMenu(GtkWidget *treeview, gpointer userdata)
{
    view_popup_menu(treeview, NULL, userdata);
    return TRUE;
}

static gboolean view_onButtonPressed(GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        view_popup_menu(treeview, event, userdata);
        return TRUE;
    }

    return FALSE;
}

struct state_find_pos {
    int offset;
    int range;
    int pos;
    const gchar *name;
};

static gboolean trg_state_selector_find_pos_foreach(GtkTreeModel *model, GtkTreePath *path,
                                                    GtkTreeIter *iter, gpointer data)
{
    struct state_find_pos *args = (struct state_find_pos *)data;
    gchar *name;
    gboolean res;

    if (args->pos < args->offset) {
        args->pos++;
        return FALSE;
    } else if (args->range >= 0 && args->pos > args->offset + args->range - 1) {
        return TRUE;
    }

    gtk_tree_model_get(model, iter, STATE_SELECTOR_NAME, &name, -1);
    res = g_strcmp0(name, args->name) >= 0;
    g_free(name);

    if (!res)
        args->pos++;

    return res;
}

static void trg_state_selector_insert(TrgStateSelector *s, int offset, gint range,
                                      const gchar *name, GtkTreeIter *iter)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(s));

    struct state_find_pos args;
    args.offset = offset;
    args.pos = 0;
    args.range = range;
    args.name = name;

    gtk_tree_model_foreach(model, trg_state_selector_find_pos_foreach, &args);
    gtk_list_store_insert(GTK_LIST_STORE(model), iter, args.pos);
}

void trg_state_selector_update(TrgStateSelector *s, guint whatsChanged)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(s));
    TrgClient *client = s->client;
    gint64 updateSerial = trg_client_get_serial(client);
    GList *torrentItemRefs;
    GtkTreeIter torrentIter, iter;
    GList *trackersList, *trackerItem, *li;
    GtkTreeRowReference *rr;
    GtkTreePath *path;
    GtkTreeModel *torrentModel;
    gpointer result;
    struct cruft_remove_args cruft;

    if (!trg_client_is_connected(client))
        return;

    torrentItemRefs = g_hash_table_get_values(trg_client_get_torrent_table(client));

    for (li = torrentItemRefs; li; li = g_list_next(li)) {
        JsonObject *t = NULL;
        rr = (GtkTreeRowReference *)li->data;
        path = gtk_tree_row_reference_get_path(rr);
        torrentModel = gtk_tree_row_reference_get_model(rr);

        if (path) {
            if (gtk_tree_model_get_iter(torrentModel, &torrentIter, path)) {
                gtk_tree_model_get(torrentModel, &torrentIter, TORRENT_COLUMN_JSON, &t, -1);
            }
            gtk_tree_path_free(path);
        }

        if (!t)
            continue;

        if (s->showTrackers && (whatsChanged & TORRENT_UPDATE_ADDREMOVE)) {
            trackersList = json_array_get_elements(torrent_get_tracker_stats(t));
            for (trackerItem = trackersList; trackerItem; trackerItem = g_list_next(trackerItem)) {
                JsonObject *tracker = json_node_get_object((JsonNode *)trackerItem->data);
                const gchar *announceUrl = tracker_stats_get_announce(tracker);
                gchar *announceHost = trg_gregex_get_first(s->urlHostRegex, announceUrl);

                if (!announceHost)
                    continue;

                result = g_hash_table_lookup(s->trackers, announceHost);

                if (result) {
                    trg_state_selector_update_dynamic_filter(model, (GtkTreeRowReference *)result,
                                                             updateSerial);
                    g_free(announceHost);
                } else {
                    if (s->dirsFirst) {
                        trg_state_selector_insert(
                            s, s->n_categories + g_hash_table_size(s->directories), -1,
                            announceHost, &iter);
                    } else {
                        trg_state_selector_insert(s, s->n_categories,
                                                  g_hash_table_size(s->trackers), announceHost,
                                                  &iter);
                    }
                    gtk_list_store_set(GTK_LIST_STORE(model), &iter, STATE_SELECTOR_ICON,
                                       "network-workgroup", STATE_SELECTOR_NAME, announceHost,
                                       STATE_SELECTOR_SERIAL, updateSerial, STATE_SELECTOR_COUNT, 1,
                                       STATE_SELECTOR_BIT, FILTER_FLAG_TRACKER,
                                       STATE_SELECTOR_INDEX, 0, -1);
                    g_hash_table_insert(s->trackers, announceHost,
                                        quick_tree_ref_new(model, &iter));
                }
            }
            g_list_free(trackersList);
        }

        if (s->showDirs
            && ((whatsChanged & TORRENT_UPDATE_ADDREMOVE)
                || (whatsChanged & TORRENT_UPDATE_PATH_CHANGE))) {
            gchar *dir;
            gtk_tree_model_get(torrentModel, &torrentIter, TORRENT_COLUMN_DOWNLOADDIR_SHORT, &dir,
                               -1);

            result = g_hash_table_lookup(s->directories, dir);
            if (result) {
                trg_state_selector_update_dynamic_filter(model, (GtkTreeRowReference *)result,
                                                         updateSerial);
            } else {
                if (s->dirsFirst) {
                    trg_state_selector_insert(s, s->n_categories, g_hash_table_size(s->directories),
                                              dir, &iter);
                } else {
                    trg_state_selector_insert(s, s->n_categories + g_hash_table_size(s->trackers),
                                              -1, dir, &iter);
                }
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, STATE_SELECTOR_ICON, "folder",
                                   STATE_SELECTOR_NAME, dir, STATE_SELECTOR_SERIAL, updateSerial,
                                   STATE_SELECTOR_BIT, FILTER_FLAG_DIR, STATE_SELECTOR_COUNT, 1,
                                   STATE_SELECTOR_INDEX, 0, -1);
                g_hash_table_insert(s->directories, g_strdup(dir),
                                    quick_tree_ref_new(model, &iter));
            }

            g_free(dir);
        }
    }

    g_list_free(torrentItemRefs);

    cruft.serial = trg_client_get_serial(client);

    if (s->showTrackers && ((whatsChanged & TORRENT_UPDATE_ADDREMOVE))) {
        cruft.table = s->trackers;
        g_hash_table_foreach_remove(s->trackers, trg_state_selector_remove_cruft, &cruft);
    }

    if (s->showDirs
        && ((whatsChanged & TORRENT_UPDATE_ADDREMOVE)
            || (whatsChanged & TORRENT_UPDATE_PATH_CHANGE))) {
        cruft.table = s->directories;
        g_hash_table_foreach_remove(s->directories, trg_state_selector_remove_cruft, &cruft);
    }
}

void trg_state_selector_set_show_dirs(TrgStateSelector *s, gboolean show)
{
    s->showDirs = show;
    if (!show)
        g_hash_table_remove_all(s->directories);
    else
        trg_state_selector_update(s, TORRENT_UPDATE_PATH_CHANGE);
}

static void on_torrents_state_change(TrgTorrentModel *model, guint whatsChanged, gpointer data)
{
    TrgStateSelector *selector = TRG_STATE_SELECTOR(data);
    trg_state_selector_update(selector, whatsChanged);

    if ((whatsChanged & TORRENT_UPDATE_ADDREMOVE) || (whatsChanged & TORRENT_UPDATE_STATE_CHANGE))
        trg_state_selector_stats_update(selector, trg_torrent_model_get_stats(model));
}

void trg_state_selector_set_show_trackers(TrgStateSelector *s, gboolean show)
{
    s->showTrackers = show;
    if (!show)
        g_hash_table_remove_all(s->trackers);
    else
        trg_state_selector_update(s, TORRENT_UPDATE_ADDREMOVE);
}

void trg_state_selector_set_directories_first(TrgStateSelector *s, gboolean _dirsFirst)
{
    s->dirsFirst = _dirsFirst;
    g_hash_table_remove_all(s->directories);
    g_hash_table_remove_all(s->trackers);
    trg_state_selector_update(s, TORRENT_UPDATE_ADDREMOVE);
}

static void trg_state_selector_add_state(TrgStateSelector *selector, GtkTreeIter *iter, gint pos,
                                         gchar *icon, gchar *name, guint32 flag,
                                         GtkTreeRowReference **rr)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(selector)));

    if (pos < 0)
        gtk_list_store_append(selector->store, iter);
    else
        gtk_list_store_insert(selector->store, iter, pos);

    gtk_list_store_set(model, iter, STATE_SELECTOR_ICON, icon, STATE_SELECTOR_NAME, name,
                       STATE_SELECTOR_BIT, flag, STATE_SELECTOR_INDEX,
                       gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL) - 1, -1);

    if (rr)
        *rr = quick_tree_ref_new(GTK_TREE_MODEL(model), iter);

    selector->n_categories++;
}

static void remove_row_ref_and_free(GtkTreeRowReference *rr)
{
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    GtkTreeIter iter;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    gtk_tree_path_free(path);
    gtk_tree_row_reference_free(rr);
}

static void trg_state_selector_update_stat(GtkTreeRowReference *rr, gint count)
{
    if (rr) {
        GValue gvalue = G_VALUE_INIT;
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
        GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);

        gtk_tree_model_get_iter(model, &iter, path);

        g_value_init(&gvalue, G_TYPE_INT);
        g_value_set_int(&gvalue, count);
        gtk_list_store_set_value(GTK_LIST_STORE(model), &iter, STATE_SELECTOR_COUNT, &gvalue);

        gtk_tree_path_free(path);
    }
}

void trg_state_selector_stats_update(TrgStateSelector *s, trg_torrent_model_update_stats *stats)
{
    GtkTreeIter iter;
    if (stats->error > 0 && !s->error_rr) {
        trg_state_selector_add_state(s, &iter, s->n_categories - 1, "dialog-warning", _("Error"),
                                     TORRENT_FLAG_ERROR, &s->error_rr);

    } else if (stats->error < 1 && s->error_rr) {
        remove_row_ref_and_free(s->error_rr);
        s->error_rr = NULL;
        s->n_categories--;
    }

    trg_state_selector_update_stat(s->all_rr, stats->count);
    trg_state_selector_update_stat(s->down_rr, stats->down);
    trg_state_selector_update_stat(s->seeding_rr, stats->seeding);
    trg_state_selector_update_stat(s->error_rr, stats->error);
    trg_state_selector_update_stat(s->paused_rr, stats->paused);
    trg_state_selector_update_stat(s->complete_rr, stats->complete);
    trg_state_selector_update_stat(s->incomplete_rr, stats->incomplete);
    trg_state_selector_update_stat(s->active_rr, stats->active);
    trg_state_selector_update_stat(s->checking_rr, stats->checking);
    trg_state_selector_update_stat(s->down_wait_rr, stats->down_wait);
    trg_state_selector_update_stat(s->seed_wait_rr, stats->seed_wait);
}

void trg_state_selector_disconnect(TrgStateSelector *s)
{

    if (s->error_rr) {
        remove_row_ref_and_free(s->error_rr);
        s->error_rr = NULL;
        s->n_categories--;
    }

    g_hash_table_remove_all(s->trackers);
    g_hash_table_remove_all(s->directories);

    trg_state_selector_update_stat(s->all_rr, -1);
    trg_state_selector_update_stat(s->down_rr, -1);
    trg_state_selector_update_stat(s->seeding_rr, -1);
    trg_state_selector_update_stat(s->error_rr, -1);
    trg_state_selector_update_stat(s->paused_rr, -1);
    trg_state_selector_update_stat(s->complete_rr, -1);
    trg_state_selector_update_stat(s->incomplete_rr, -1);
    trg_state_selector_update_stat(s->active_rr, -1);
    trg_state_selector_update_stat(s->checking_rr, -1);
}

static void trg_state_selector_init(TrgStateSelector *self)
{
}

TrgStateSelector *trg_state_selector_new(TrgClient *client, TrgTorrentModel *tmodel)
{
    TrgStateSelector *selector = g_object_new(TRG_TYPE_STATE_SELECTOR, "client", client, NULL);
    g_signal_connect(tmodel, "torrents-state-change", G_CALLBACK(on_torrents_state_change),
                     selector);
    return selector;
}

static GObject *trg_state_selector_constructor(GType type, guint n_construct_properties,
                                               GObjectConstructParam *construct_params)
{
    GObject *object;
    TrgStateSelector *selector;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeIter iter;
    gint index;
    GtkTreeSelection *selection;

    object = G_OBJECT_CLASS(trg_state_selector_parent_class)
                 ->constructor(type, n_construct_properties, construct_params);

    selector = TRG_STATE_SELECTOR(object);

    selector->urlHostRegex = trg_uri_host_regex_new();
    selector->trackers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                               (GDestroyNotify)remove_row_ref_and_free);
    selector->directories = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                                  (GDestroyNotify)remove_row_ref_and_free);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(object), FALSE);

    column = gtk_tree_view_column_new();

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    g_object_set(renderer, "stock-size", 4, NULL);
    gtk_tree_view_column_set_attributes(column, renderer, "icon-name", STATE_SELECTOR_ICON, NULL);

    renderer = trg_cell_renderer_counter_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer, "state-label", STATE_SELECTOR_NAME,
                                        "state-count", STATE_SELECTOR_COUNT, NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(object), column);

    store = selector->store
        = gtk_list_store_new(STATE_SELECTOR_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
                             G_TYPE_UINT, G_TYPE_INT64, G_TYPE_UINT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(object), GTK_TREE_MODEL(store));

    trg_state_selector_add_state(selector, &iter, -1, "help-about", _("All"), 0, &selector->all_rr);
    trg_state_selector_add_state(selector, &iter, -1, "go-down", _("Downloading"),
                                 TORRENT_FLAG_DOWNLOADING, &selector->down_rr);
    trg_state_selector_add_state(selector, &iter, -1, "media-seek-backward", _("Queue Down"),
                                 TORRENT_FLAG_DOWNLOADING_WAIT, &selector->down_wait_rr);
    trg_state_selector_add_state(selector, &iter, -1, "go-up", _("Seeding"), TORRENT_FLAG_SEEDING,
                                 &selector->seeding_rr);
    trg_state_selector_add_state(selector, &iter, -1, "media-seek-forward", _("Queue Up"),
                                 TORRENT_FLAG_SEEDING_WAIT, &selector->seed_wait_rr);
    trg_state_selector_add_state(selector, &iter, -1, "media-playback-pause", _("Paused"),
                                 TORRENT_FLAG_PAUSED, &selector->paused_rr);
    trg_state_selector_add_state(selector, &iter, -1, "trg-gtk-apply", _("Complete"),
                                 TORRENT_FLAG_COMPLETE, &selector->complete_rr);
    trg_state_selector_add_state(selector, &iter, -1, "edit-select-all", _("Incomplete"),
                                 TORRENT_FLAG_INCOMPLETE, &selector->incomplete_rr);
    trg_state_selector_add_state(selector, &iter, -1, "network-workgroup", _("Active"),
                                 TORRENT_FLAG_ACTIVE, &selector->active_rr);
    trg_state_selector_add_state(selector, &iter, -1, "view-refresh", _("Checking"),
                                 TORRENT_FLAG_CHECKING_ANY, &selector->checking_rr);
    trg_state_selector_add_state(selector, &iter, -1, NULL, NULL, 0, NULL);

    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(object), TRUE);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(object));

    g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(state_selection_changed), object);
    g_signal_connect(object, "button-press-event", G_CALLBACK(view_onButtonPressed), NULL);
    g_signal_connect(object, "popup-menu", G_CALLBACK(view_onPopupMenu), NULL);

    gtk_tree_view_set_search_column(GTK_TREE_VIEW(object), STATE_SELECTOR_NAME);

    index = trg_prefs_get_int(selector->prefs, TRG_PREFS_STATE_SELECTOR_LAST, TRG_PREFS_GLOBAL);
    if (index > 0 && gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, index)) {
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(object));
        gtk_tree_selection_select_iter(selection, &iter);
    }

    selector->showDirs
        = trg_prefs_get_bool(selector->prefs, TRG_PREFS_KEY_FILTER_DIRS, TRG_PREFS_GLOBAL);
    selector->showTrackers
        = trg_prefs_get_bool(selector->prefs, TRG_PREFS_KEY_FILTER_TRACKERS, TRG_PREFS_GLOBAL);
    selector->dirsFirst
        = trg_prefs_get_bool(selector->prefs, TRG_PREFS_KEY_DIRECTORIES_FIRST, TRG_PREFS_GLOBAL);

    return object;
}

void trg_state_selector_set_queues_enabled(TrgStateSelector *s, gboolean enabled)
{
    GtkTreeIter iter;

    if (enabled) {
        trg_state_selector_add_state(s, &iter, 2, "media-seek-backward", _("Queue Down"),
                                     TORRENT_FLAG_DOWNLOADING_WAIT, &s->down_wait_rr);
        trg_state_selector_add_state(s, &iter, 4, "media-seek-forward", _("Queue Up"),
                                     TORRENT_FLAG_SEEDING_WAIT, &s->seed_wait_rr);
    } else {
        remove_row_ref_and_free(s->seed_wait_rr);
        remove_row_ref_and_free(s->down_wait_rr);
        s->down_wait_rr = NULL;
        s->seed_wait_rr = NULL;
        s->n_categories -= 2;
    }
}

static void trg_state_selector_get_property(GObject *object, guint property_id, GValue *value,
                                            GParamSpec *pspec)
{
    TrgStateSelector *self = TRG_STATE_SELECTOR(object);
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_object(value, self->client);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_state_selector_set_property(GObject *object, guint prop_id, const GValue *value,
                                            GParamSpec *pspec G_GNUC_UNUSED)
{
    TrgStateSelector *self = TRG_STATE_SELECTOR(object);
    switch (prop_id) {
    case PROP_CLIENT:
        self->client = g_value_get_object(value);
        self->prefs = trg_client_get_prefs(self->client);
        break;
    }
}

static void trg_state_selector_class_init(TrgStateSelectorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->constructor = trg_state_selector_constructor;
    object_class->set_property = trg_state_selector_set_property;
    object_class->get_property = trg_state_selector_get_property;

    signals[SELECTOR_STATE_CHANGED]
        = g_signal_new("torrent-state-changed", G_TYPE_FROM_CLASS(object_class),
                       G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                       g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

    g_object_class_install_property(
        object_class, PROP_CLIENT,
        g_param_spec_object("client", "Client", "Client", TRG_TYPE_CLIENT,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}
