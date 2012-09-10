/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION( 3, 0, 0 )
#include <gdk/gdkkeysyms-compat.h>
#endif
#include <curl/curl.h>
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif
#ifdef HAVE_LIBAPPINDICATOR
#include <libappindicator/app-indicator.h>
#endif

#include "trg-client.h"
#include "json.h"
#include "util.h"
#include "requests.h"
#include "session-get.h"
#include "torrent.h"
#include "protocol-constants.h"
#include "remote-exec.h"

#include "trg-main-window.h"
#include "trg-icons.h"
#include "trg-about-window.h"
#include "trg-tree-view.h"
#include "trg-prefs.h"
#include "trg-sortable-filtered-model.h"
#include "trg-torrent-model.h"
#include "trg-torrent-tree-view.h"
#include "trg-peers-model.h"
#include "trg-peers-tree-view.h"
#include "trg-files-tree-view.h"
#include "trg-files-model.h"
#include "trg-trackers-tree-view.h"
#include "trg-trackers-model.h"
#include "trg-state-selector.h"
#include "trg-torrent-graph.h"
#include "trg-torrent-move-dialog.h"
#include "trg-torrent-props-dialog.h"
#include "trg-torrent-add-url-dialog.h"
#include "trg-torrent-add-dialog.h"
#include "trg-toolbar.h"
#include "trg-menu-bar.h"
#include "trg-status-bar.h"
#include "trg-stats-dialog.h"
#include "trg-remote-prefs-dialog.h"
#include "trg-preferences-dialog.h"

/* The rather large main window class, which glues everything together. */

static void update_selected_torrent_notebook(TrgMainWindow * win,
                                             gint mode, gint64 id);
#ifdef HAVE_LIBNOTIFY
static void torrent_event_notification(TrgTorrentModel * model,
                                       gchar * icon, gchar * desc,
                                       gint tmout, gchar * prefKey,
                                       GtkTreeIter * iter, gpointer data);
#endif
static void connchange_whatever_statusicon(TrgMainWindow * win,
                                           gboolean connected);
static void update_whatever_statusicon(TrgMainWindow * win,
                                       trg_torrent_model_update_stats *
                                       stats);
static void on_torrent_completed(TrgTorrentModel * model,
                                 GtkTreeIter * iter, gpointer data);
static void on_torrent_added(TrgTorrentModel * model, GtkTreeIter * iter,
                             gpointer data);
static gboolean delete_event(GtkWidget * w, GdkEvent * event,
                             gpointer data);
static void destroy_window(TrgMainWindow * win,
                           gpointer data G_GNUC_UNUSED);
static void torrent_tv_onRowActivated(GtkTreeView * treeview,
                                      GtkTreePath * path,
                                      GtkTreeViewColumn * col,
                                      gpointer userdata);
static void add_url_cb(GtkWidget * w, gpointer data);
static void add_cb(GtkWidget * w, gpointer data);
static void disconnect_cb(GtkWidget * w, gpointer data);
static void open_local_prefs_cb(GtkWidget * w G_GNUC_UNUSED,
                                TrgMainWindow * win);
static void open_remote_prefs_cb(GtkWidget * w G_GNUC_UNUSED,
                                 TrgMainWindow * win);
static TrgToolbar *trg_main_window_toolbar_new(TrgMainWindow * win);
static void verify_cb(GtkWidget * w, TrgMainWindow * win);
static void reannounce_cb(GtkWidget * w, TrgMainWindow * win);
static void pause_cb(GtkWidget * w, TrgMainWindow * win);
static void resume_cb(GtkWidget * w, TrgMainWindow * win);
static void remove_cb(GtkWidget * w, TrgMainWindow * win);
static void resume_all_cb(GtkWidget * w, TrgMainWindow * win);
static void pause_all_cb(GtkWidget * w, TrgMainWindow * win);
static void move_cb(GtkWidget * w, TrgMainWindow * win);
static void delete_cb(GtkWidget * w, TrgMainWindow * win);
static void open_props_cb(GtkWidget * w, TrgMainWindow * win);
static gint confirm_action_dialog(GtkWindow * gtk_win,
                                  GtkTreeSelection * selection,
                                  const gchar * question_single,
                                  const gchar * question_multi,
                                  const gchar * action_stock);
static void view_stats_toggled_cb(GtkWidget * w, gpointer data);
static void view_states_toggled_cb(GtkCheckMenuItem * w,
                                   TrgMainWindow * win);
static void view_notebook_toggled_cb(GtkCheckMenuItem * w,
                                     TrgMainWindow * win);
static GtkWidget *trg_main_window_notebook_new(TrgMainWindow * win);
static gboolean on_session_get_timer(gpointer data);
static gboolean on_session_get(gpointer data);
static gboolean on_torrent_get(gpointer data, int mode);
static gboolean on_torrent_get_first(gpointer data);
static gboolean on_torrent_get_active(gpointer data);
static gboolean on_torrent_get_update(gpointer data);
static gboolean on_torrent_get_interactive(gpointer data);
static gboolean trg_session_update_timerfunc(gpointer data);
static gboolean trg_update_torrents_timerfunc(gpointer data);
static void open_about_cb(GtkWidget * w, GtkWindow * parent);
static gboolean trg_torrent_tree_view_visible_func(GtkTreeModel * model,
                                                   GtkTreeIter * iter,
                                                   gpointer data);
static TrgTorrentTreeView
    * trg_main_window_torrent_tree_view_new(TrgMainWindow * win,
                                            GtkTreeModel * model);
static gboolean trg_dialog_error_handler(TrgMainWindow * win,
                                         trg_response * response);
static gboolean torrent_selection_changed(GtkTreeSelection * selection,
                                          TrgMainWindow * win);
static void trg_main_window_torrent_scrub(TrgMainWindow * win);
static void entry_filter_changed_cb(GtkWidget * w, TrgMainWindow * win);
static void torrent_state_selection_changed(TrgStateSelector * selector,
                                            guint flag, gpointer data);
static void trg_main_window_conn_changed(TrgMainWindow * win,
                                         gboolean connected);
static void trg_main_window_get_property(GObject * object,
                                         guint property_id, GValue * value,
                                         GParamSpec * pspec);
static void trg_main_window_set_property(GObject * object,
                                         guint property_id,
                                         const GValue * value,
                                         GParamSpec * pspec);
static void quit_cb(GtkWidget * w, gpointer data);
static TrgMenuBar *trg_main_window_menu_bar_new(TrgMainWindow * win);
static void status_icon_activated(GtkStatusIcon * icon,
                                  TrgMainWindow * win);
static gboolean trg_status_icon_popup_menu_cb(GtkStatusIcon * icon,
                                              TrgMainWindow * win);
static gboolean status_icon_button_press_event(GtkStatusIcon * icon,
                                               GdkEventButton * event,
                                               TrgMainWindow * win);
static void clear_filter_entry_cb(GtkEntry * entry,
                                  GtkEntryIconPosition icon_pos,
                                  GdkEvent * event, gpointer user_data);
static GtkWidget *trg_imagemenuitem_new(GtkMenuShell * shell,
                                        const gchar * text, char *stock_id,
                                        gboolean sensitive, GCallback cb,
                                        gpointer cbdata);
static void set_limit_cb(GtkWidget * w, TrgMainWindow * win);
static GtkWidget *limit_item_new(TrgMainWindow * win, GtkWidget * menu,
                                 gint64 currentLimit, gfloat limit);
static GtkWidget *limit_menu_new(TrgMainWindow * win, gchar * title,
                                 gchar * enabledKey, gchar * speedKey,
                                 JsonArray * ids);
static void trg_torrent_tv_view_menu(GtkWidget * treeview,
                                     GdkEventButton * event,
                                     TrgMainWindow * win);
static GtkMenu *trg_status_icon_view_menu(TrgMainWindow * win,
                                          const gchar * msg);
static gboolean torrent_tv_button_pressed_cb(GtkWidget * treeview,
                                             GdkEventButton * event,
                                             gpointer userdata);
static gboolean torrent_tv_popup_menu_cb(GtkWidget * treeview,
                                         gpointer userdata);
static void trg_main_window_set_hidden_to_tray(TrgMainWindow * win,
                                               gboolean hidden);
static gboolean is_ready_for_torrent_action(TrgMainWindow * win);
static gboolean window_state_event(TrgMainWindow * win,
                                   GdkEventWindowState * event,
                                   gpointer trayIcon);


G_DEFINE_TYPE(TrgMainWindow, trg_main_window, GTK_TYPE_WINDOW)
struct _TrgMainWindowPrivate {
    TrgClient *client;
    TrgToolbar *toolBar;
    TrgMenuBar *menuBar;

    TrgStatusBar *statusBar;
    GtkWidget *iconStatusItem, *iconDownloadingItem, *iconSeedingItem,
        *iconSepItem;
#ifdef HAVE_LIBAPPINDICATOR
    AppIndicator *appIndicator;
#endif
    GtkMenu *iconMenu;
    GtkStatusIcon *statusIcon;
    TrgStateSelector *stateSelector;
    GtkWidget *stateSelectorScroller;
    TrgGeneralPanel *genDetails;
    GtkWidget *notebook;

    TrgTorrentModel *torrentModel;
    TrgTorrentTreeView *torrentTreeView;
    GtkTreeModel *filteredTorrentModel;
    GtkTreeModel *sortedTorrentModel;
    gint selectedTorrentId;

    TrgTrackersModel *trackersModel;
    TrgTrackersTreeView *trackersTreeView;

    TrgFilesModel *filesModel;
    TrgFilesTreeView *filesTreeView;

    TrgPeersModel *peersModel;
    TrgPeersTreeView *peersTreeView;

#if TRG_WITH_GRAPH
    TrgTorrentGraph *graph;
#endif
    gint graphNotebookIndex;

    GtkWidget *hpaned, *vpaned;
    GtkWidget *filterEntry;

    gboolean hidden;
    gint width, height;
    guint timerId;
    guint sessionTimerId;
    gboolean min_on_start;
    gboolean queuesEnabled;

    gchar **args;
};

enum {
    PROP_0, PROP_CLIENT, PROP_MINIMISE_ON_START
};

static void reset_connect_args(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    if (priv->args) {
        g_strfreev(priv->args);
        priv->args = NULL;
    }
}

static void trg_main_window_init(TrgMainWindow * self)
{
    self->priv =
        G_TYPE_INSTANCE_GET_PRIVATE(self, TRG_TYPE_MAIN_WINDOW,
                                    TrgMainWindowPrivate);
}

gint trg_mw_get_selected_torrent_id(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    return priv->selectedTorrentId;
}

static void
update_selected_torrent_notebook(TrgMainWindow * win, gint mode, gint64 id)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *client = priv->client;
    gint64 serial = trg_client_get_serial(client);
    JsonObject *t;
    GtkTreeIter iter;

    if (id >= 0
        && get_torrent_data(trg_client_get_torrent_table(client), id, &t,
                            &iter)) {
        trg_toolbar_torrent_actions_sensitive(priv->toolBar, TRUE);
        trg_menu_bar_torrent_actions_sensitive(priv->menuBar, TRUE);
        trg_general_panel_update(priv->genDetails, t, &iter);
        trg_trackers_model_update(priv->trackersModel, serial, t, mode);
        trg_files_model_update(priv->filesModel,
                               GTK_TREE_VIEW(priv->filesTreeView),
                               serial, t, mode);
        trg_peers_model_update(priv->peersModel,
                               TRG_TREE_VIEW(priv->peersTreeView),
                               serial, t, mode);
    } else {
        trg_main_window_torrent_scrub(win);
    }

    priv->selectedTorrentId = id;
}

#ifdef HAVE_LIBNOTIFY
static void
torrent_event_notification(TrgTorrentModel * model,
                           gchar * icon, gchar * desc,
                           gint tmout, gchar * prefKey,
                           GtkTreeIter * iter, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    gchar *name;
    NotifyNotification *notify;

    if (!trg_prefs_get_bool(prefs, prefKey, TRG_PREFS_NOFLAGS))
        return;

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter, TORRENT_COLUMN_NAME,
                       &name, -1);

    notify = notify_notification_new(name, desc, icon
#if !defined(NOTIFY_VERSION_MINOR) || (NOTIFY_VERSION_MAJOR == 0 && NOTIFY_VERSION_MINOR < 7)
                                     , NULL
#endif
        );

#if !defined(NOTIFY_VERSION_MINOR) || (NOTIFY_VERSION_MAJOR == 0 && NOTIFY_VERSION_MINOR < 7)
    if (priv->statusIcon && gtk_status_icon_is_embedded(priv->statusIcon))
        notify_notification_attach_to_status_icon(notify,
                                                  priv->statusIcon);
#endif

    notify_notification_set_urgency(notify, NOTIFY_URGENCY_LOW);
    notify_notification_set_timeout(notify, tmout);

    g_free(name);

    notify_notification_show(notify, NULL);
}
#endif

static void
on_torrent_completed(TrgTorrentModel * model,
                     GtkTreeIter * iter, gpointer data)
{
#ifdef HAVE_LIBNOTIFY
    torrent_event_notification(model, GTK_STOCK_APPLY,
                               _("This torrent has completed."),
                               TORRENT_COMPLETE_NOTIFY_TMOUT,
                               TRG_PREFS_KEY_COMPLETE_NOTIFY, iter, data);
#endif
}

static void
on_torrent_added(TrgTorrentModel * model, GtkTreeIter * iter,
                 gpointer data)
{
#ifdef HAVE_LIBNOTIFY
    torrent_event_notification(model, GTK_STOCK_ADD,
                               _("This torrent has been added."),
                               TORRENT_ADD_NOTIFY_TMOUT,
                               TRG_PREFS_KEY_ADD_NOTIFY, iter, data);
#endif
}

static gboolean
delete_event(GtkWidget * w, GdkEvent * event G_GNUC_UNUSED,
             gpointer data G_GNUC_UNUSED)
{
    return FALSE;
}

static void
destroy_window(TrgMainWindow * win, gpointer data G_GNUC_UNUSED)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    trg_prefs_set_int(prefs, TRG_PREFS_KEY_WINDOW_HEIGHT, priv->height,
                      TRG_PREFS_GLOBAL);
    trg_prefs_set_int(prefs, TRG_PREFS_KEY_WINDOW_WIDTH, priv->width,
                      TRG_PREFS_GLOBAL);
    trg_prefs_set_int(prefs, TRG_PREFS_KEY_NOTEBOOK_PANED_POS,
                      gtk_paned_get_position(GTK_PANED(priv->vpaned)),
                      TRG_PREFS_GLOBAL);
    trg_prefs_set_int(prefs, TRG_PREFS_KEY_STATES_PANED_POS,
                      gtk_paned_get_position(GTK_PANED(priv->hpaned)),
                      TRG_PREFS_GLOBAL);

    trg_tree_view_persist(TRG_TREE_VIEW(priv->peersTreeView),
                          TRG_TREE_VIEW_PERSIST_SORT |
                          TRG_TREE_VIEW_PERSIST_LAYOUT);
    trg_tree_view_persist(TRG_TREE_VIEW(priv->filesTreeView),
                          TRG_TREE_VIEW_PERSIST_SORT |
                          TRG_TREE_VIEW_PERSIST_LAYOUT);
    trg_tree_view_persist(TRG_TREE_VIEW(priv->torrentTreeView),
                          TRG_TREE_VIEW_PERSIST_SORT |
                          TRG_TREE_VIEW_SORTABLE_PARENT |
                          (trg_prefs_get_int
                           (prefs, TRG_PREFS_KEY_STYLE,
                            TRG_PREFS_GLOBAL) ==
                           TRG_STYLE_CLASSIC ? TRG_TREE_VIEW_PERSIST_LAYOUT
                           : 0));
    trg_tree_view_persist(TRG_TREE_VIEW(priv->trackersTreeView),
                          TRG_TREE_VIEW_PERSIST_SORT |
                          TRG_TREE_VIEW_PERSIST_LAYOUT);
    trg_prefs_save(prefs);

#if ! GTK_CHECK_VERSION( 3, 0, 0 )
    gtk_main_quit();
#else
    g_application_quit (g_application_get_default ());
#endif
}

static void open_props_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgTorrentPropsDialog *dialog;

    if (priv->selectedTorrentId < 0)
        return;

    dialog = trg_torrent_props_dialog_new(GTK_WINDOW(win),
                                          priv->torrentTreeView,
                                          priv->torrentModel,
                                          priv->client);

    gtk_widget_show_all(GTK_WIDGET(dialog));
}

static void
torrent_tv_onRowActivated(GtkTreeView * treeview,
                          GtkTreePath * path G_GNUC_UNUSED,
                          GtkTreeViewColumn *
                          col G_GNUC_UNUSED, gpointer userdata)
{
    open_props_cb(GTK_WIDGET(treeview), userdata);
}

static void add_url_cb(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    TrgTorrentAddUrlDialog *dlg = trg_torrent_add_url_dialog_new(win,
                                                                 priv->
                                                                 client);
    gtk_widget_show_all(GTK_WIDGET(dlg));
}

static void add_cb(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        trg_torrent_add_dialog(win, priv->client);
}

static void pause_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        dispatch_async(priv->client,
                       torrent_pause(build_json_id_array
                                     (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void pause_all_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        dispatch_async(priv->client, torrent_pause(NULL),
                       on_generic_interactive_action, win);
}

gint trg_add_from_filename(TrgMainWindow * win, gchar ** uris)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *client = priv->client;
    TrgPrefs *prefs = trg_client_get_prefs(client);
    GSList *filesList = NULL;
    int i;

    if (!trg_client_is_connected(client)) {
        g_strfreev(uris);
        return EXIT_SUCCESS;
    }

    if (uris)
        for (i = 0; uris[i]; i++)
            if (uris[i])
                filesList = g_slist_append(filesList, uris[i]);

    g_free(uris);

    if (!filesList)
        return EXIT_SUCCESS;

    if (trg_prefs_get_bool(prefs, TRG_PREFS_KEY_ADD_OPTIONS_DIALOG,
                           TRG_PREFS_GLOBAL)) {
        TrgTorrentAddDialog *dialog =
            trg_torrent_add_dialog_new(win, client,
                                       filesList);

        gtk_widget_show_all(GTK_WIDGET(dialog));
    } else {
        struct add_torrent_threadfunc_args *args =
            g_new0(struct add_torrent_threadfunc_args, 1);
        args->list = filesList;
        args->cb_data = win;
        args->client = client;
        args->extraArgs = FALSE;
        args->flags = trg_prefs_get_add_flags(prefs);

        launch_add_thread(args);
    }

    return EXIT_SUCCESS;
}

static void resume_all_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        dispatch_async(priv->client, torrent_start(NULL),
                       on_generic_interactive_action, win);
}

static void resume_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        dispatch_async(priv->client,
                       torrent_start(build_json_id_array
                                     (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void disconnect_cb(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    trg_client_inc_connid(priv->client);
    trg_main_window_conn_changed(TRG_MAIN_WINDOW(data), FALSE);
    trg_status_bar_reset(priv->statusBar);
}

void connect_cb(GtkWidget * w, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    JsonObject *currentProfile = trg_prefs_get_profile(prefs);
    JsonObject *profile = NULL;
    GtkWidget *dialog;
    int populate_result;

    if (w)
        profile = (JsonObject *) g_object_get_data(G_OBJECT(w), "profile");

    if (trg_client_is_connected(priv->client))
        disconnect_cb(NULL, data);

    if (profile && currentProfile != profile)
        trg_prefs_set_profile(prefs, profile);
    else
        trg_prefs_profile_change_emit_signal(prefs);

    populate_result = trg_client_populate_with_settings(priv->client);

    if (populate_result < 0) {
        gchar *msg;

        switch (populate_result) {
        case TRG_NO_HOSTNAME_SET:
            msg = _("No host specified. Set it in the preferences dialog.");
            break;
        default:
            msg = _("Unknown error getting settings");
            break;
        }

	dialog = gtk_message_dialog_new(GTK_WINDOW(data),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					(populate_result == TRG_NO_HOSTNAME_SET) ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR, 
					GTK_BUTTONS_OK,
					"%s", msg);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
        reset_connect_args(TRG_MAIN_WINDOW(data));

	if (populate_result == TRG_NO_HOSTNAME_SET)
	  open_local_prefs_cb (NULL, win);

        return;
    }

    trg_status_bar_push_connection_msg(priv->statusBar,
                                       _("Connecting..."));
    trg_client_inc_connid(priv->client);
    dispatch_async(priv->client, session_get(), on_session_get, data);
}

static void
open_local_prefs_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    GtkWidget *dlg = trg_preferences_dialog_get_instance(win,
                                                         priv->client);
    gtk_widget_show_all(dlg);
}

static void
open_remote_prefs_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        gtk_widget_show_all(GTK_WIDGET
                            (trg_remote_prefs_dialog_get_instance
                             (win, priv->client)));
}

static void
main_window_toggle_filter_dirs(GtkCheckMenuItem * w, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    if (gtk_widget_is_sensitive(GTK_WIDGET(w)))
        trg_state_selector_set_show_dirs(priv->stateSelector,
                                         gtk_check_menu_item_get_active
                                         (w));
}

static void
main_window_toggle_filter_trackers(GtkCheckMenuItem * w, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    if (gtk_widget_is_sensitive(GTK_WIDGET(w)))
        trg_state_selector_set_show_trackers(priv->stateSelector,
                                             gtk_check_menu_item_get_active
                                             (w));
}

static TrgToolbar *trg_main_window_toolbar_new(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    GObject *b_connect, *b_disconnect, *b_add, *b_resume, *b_pause;
    GObject *b_remove, *b_delete, *b_props, *b_local_prefs,
        *b_remote_prefs;

    TrgToolbar *toolBar = trg_toolbar_new(win, prefs);

    g_object_get(toolBar, "connect-button", &b_connect,
                 "disconnect-button", &b_disconnect, "add-button", &b_add,
                 "resume-button", &b_resume, "pause-button", &b_pause,
                 "delete-button", &b_delete, "remove-button", &b_remove,
                 "props-button", &b_props, "remote-prefs-button",
                 &b_remote_prefs, "local-prefs-button", &b_local_prefs,
                 NULL);

    g_signal_connect(b_connect, "clicked", G_CALLBACK(connect_cb), win);
    g_signal_connect(b_disconnect, "clicked", G_CALLBACK(disconnect_cb),
                     win);
    g_signal_connect(b_add, "clicked", G_CALLBACK(add_cb), win);
    g_signal_connect(b_resume, "clicked", G_CALLBACK(resume_cb), win);
    g_signal_connect(b_pause, "clicked", G_CALLBACK(pause_cb), win);
    g_signal_connect(b_delete, "clicked", G_CALLBACK(delete_cb), win);
    g_signal_connect(b_remove, "clicked", G_CALLBACK(remove_cb), win);
    g_signal_connect(b_props, "clicked", G_CALLBACK(open_props_cb), win);
    g_signal_connect(b_local_prefs, "clicked",
                     G_CALLBACK(open_local_prefs_cb), win);
    g_signal_connect(b_remote_prefs, "clicked",
                     G_CALLBACK(open_remote_prefs_cb), win);

    return toolBar;
}

static void reannounce_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client))
        dispatch_async(priv->client,
                       torrent_reannounce(build_json_id_array
                                          (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void verify_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (is_ready_for_torrent_action(win))
        dispatch_async(priv->client,
                       torrent_verify(build_json_id_array
                                      (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void start_now_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (is_ready_for_torrent_action(win))
        dispatch_async(priv->client,
                       torrent_start_now(build_json_id_array
                                         (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void up_queue_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (priv->queuesEnabled && is_ready_for_torrent_action(win))
        dispatch_async(priv->client,
                       torrent_queue_move_up(build_json_id_array
                                             (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void top_queue_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (priv->queuesEnabled && is_ready_for_torrent_action(win))
        dispatch_async(priv->client,
                       torrent_queue_move_top(build_json_id_array
                                              (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void
bottom_queue_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (priv->queuesEnabled && is_ready_for_torrent_action(win))
        dispatch_async(priv->client,
                       torrent_queue_move_bottom(build_json_id_array
                                                 (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static void down_queue_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (priv->queuesEnabled && is_ready_for_torrent_action(win))
        dispatch_async(priv->client,
                       torrent_queue_move_down(build_json_id_array
                                               (priv->torrentTreeView)),
                       on_generic_interactive_action, win);
}

static gint
confirm_action_dialog(GtkWindow * gtk_win,
                      GtkTreeSelection * selection,
                      const gchar * question_single,
                      const gchar * question_multi,
                      const gchar * action_stock)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(gtk_win);
    TrgMainWindowPrivate *priv = win->priv;
    gint selectCount;
    gint response;
    GtkWidget *dialog = NULL;

    selectCount = gtk_tree_selection_count_selected_rows(selection);

    if (selectCount == 1) {
        GList *list;
        GList *firstNode;
        GtkTreeIter firstIter;
        gchar *name = NULL;

        list = gtk_tree_selection_get_selected_rows(selection, NULL);
        firstNode = g_list_first(list);

        gtk_tree_model_get_iter(GTK_TREE_MODEL
                                (priv->filteredTorrentModel), &firstIter,
                                firstNode->data);
        gtk_tree_model_get(GTK_TREE_MODEL(priv->filteredTorrentModel),
                           &firstIter, TORRENT_COLUMN_NAME, &name, -1);
        g_list_foreach(list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free(list);

        dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(win),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_QUESTION,
                                                    GTK_BUTTONS_NONE,
                                                    question_single, name);
        g_free(name);
    } else if (selectCount > 1) {
        dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(win),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_QUESTION,
                                                    GTK_BUTTONS_NONE,
                                                    question_multi,
                                                    selectCount);

    } else {
        return 0;
    }

    gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
                           GTK_RESPONSE_CANCEL, action_stock,
                           GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog),
                                    GTK_RESPONSE_CANCEL);
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
                                            GTK_RESPONSE_ACCEPT,
                                            GTK_RESPONSE_CANCEL, -1);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return response;
}

static gboolean is_ready_for_torrent_action(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    return priv->selectedTorrentId >= 0
        && trg_client_is_connected(priv->client);
}

static void move_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (is_ready_for_torrent_action(win))
        gtk_widget_show_all(GTK_WIDGET
                            (trg_torrent_move_dialog_new
                             (win, priv->client, priv->torrentTreeView)));
}

static void remove_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    GtkTreeSelection *selection;
    JsonArray *ids;

    if (!is_ready_for_torrent_action(win))
        return;

    selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->torrentTreeView));
    ids = build_json_id_array(priv->torrentTreeView);

    if (confirm_action_dialog(GTK_WINDOW(win), selection, _
                              ("<big><b>Remove torrent \"%s\"?</b></big>"),
                              _("<big><b>Remove %d torrents?</b></big>"),
                              GTK_STOCK_REMOVE) == GTK_RESPONSE_ACCEPT)
        dispatch_async(priv->client, torrent_remove(ids, FALSE),
                       on_generic_interactive_action, win);
    else
        json_array_unref(ids);
}

static void delete_cb(GtkWidget * w G_GNUC_UNUSED, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    GtkTreeSelection *selection;
    JsonArray *ids;

    selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->torrentTreeView));
    ids = build_json_id_array(priv->torrentTreeView);

    if (!is_ready_for_torrent_action(win))
        return;

    if (confirm_action_dialog(GTK_WINDOW(win), selection, _
                              ("<big><b>Remove and delete torrent \"%s\"?</b></big>"),
                              _
                              ("<big><b>Remove and delete %d torrents?</b></big>"),
                              GTK_STOCK_DELETE) == GTK_RESPONSE_ACCEPT)
        dispatch_async(priv->client, torrent_remove(ids, TRUE),
                       on_delete_complete, win);
    else
        json_array_unref(ids);
}

static void view_stats_toggled_cb(GtkWidget * w, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    if (trg_client_is_connected(priv->client)) {
        TrgStatsDialog *dlg =
            trg_stats_dialog_get_instance(TRG_MAIN_WINDOW(data),
                                          priv->client);

        gtk_widget_show_all(GTK_WIDGET(dlg));
    }
}

static void
view_states_toggled_cb(GtkCheckMenuItem * w, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    trg_widget_set_visible(priv->stateSelectorScroller,
                           gtk_check_menu_item_get_active(w));
}

static void
view_notebook_toggled_cb(GtkCheckMenuItem * w, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    trg_widget_set_visible(priv->notebook,
                           gtk_check_menu_item_get_active(w));
}

#if TRG_WITH_GRAPH
static void
trg_main_window_toggle_graph_cb(GtkCheckMenuItem * w, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    if (!gtk_widget_is_sensitive(GTK_WIDGET(w))) {
        return;
    } else if (gtk_check_menu_item_get_active(w)) {
        if (priv->graphNotebookIndex < 0)
            trg_main_window_add_graph(TRG_MAIN_WINDOW(win), TRUE);
    } else if (priv->graphNotebookIndex >= 0) {
        trg_main_window_remove_graph(TRG_MAIN_WINDOW(win));
    }
}
#endif

void
trg_main_window_notebook_set_visible(TrgMainWindow * win, gboolean visible)
{
    TrgMainWindowPrivate *priv = win->priv;
    trg_widget_set_visible(priv->notebook, visible);
}

static GtkWidget *trg_main_window_notebook_new(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    GtkWidget *notebook = priv->notebook = gtk_notebook_new();
    GtkWidget *genScrolledWin = gtk_scrolled_window_new(NULL, NULL);

    priv->genDetails =
        trg_general_panel_new(GTK_TREE_MODEL(priv->torrentModel),
                              priv->client);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(genScrolledWin),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW
                                          (genScrolledWin),
                                          GTK_WIDGET(priv->genDetails));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), genScrolledWin,
                             gtk_label_new(_("General")));

    priv->trackersModel = trg_trackers_model_new();
    priv->trackersTreeView =
        trg_trackers_tree_view_new(priv->trackersModel, priv->client, win,
                                   NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             my_scrolledwin_new(GTK_WIDGET
                                                (priv->trackersTreeView)),
                             gtk_label_new(_("Trackers")));

    priv->filesModel = trg_files_model_new();
    priv->filesTreeView = trg_files_tree_view_new(priv->filesModel, win,
                                                  priv->client, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             my_scrolledwin_new(GTK_WIDGET
                                                (priv->filesTreeView)),
                             gtk_label_new(_("Files")));

    priv->peersModel = trg_peers_model_new();
    priv->peersTreeView =
        trg_peers_tree_view_new(prefs, priv->peersModel, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             my_scrolledwin_new(GTK_WIDGET
                                                (priv->peersTreeView)),
                             gtk_label_new(_("Peers")));

#if TRG_WITH_GRAPH
    if (trg_prefs_get_bool
        (prefs, TRG_PREFS_KEY_SHOW_GRAPH, TRG_PREFS_GLOBAL))
        trg_main_window_add_graph(win, FALSE);
    else
        priv->graphNotebookIndex = -1;
#endif

    return notebook;
}

gboolean on_session_set(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;

    if (response->status == CURLE_OK
        || response->status == FAIL_RESPONSE_UNSUCCESSFUL)
        trg_client_update_session(priv->client, on_session_get,
                                  response->cb_data);

    trg_dialog_error_handler(TRG_MAIN_WINDOW(response->cb_data), response);
    trg_response_free(response);

    return FALSE;
}

static gboolean
hasEnabledChanged(JsonObject * a, JsonObject * b, const gchar * key)
{
    return json_object_get_boolean_member(a, key) !=
        json_object_get_boolean_member(b, key);
}

static gboolean on_session_get_timer(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    on_session_get(data);

    priv->sessionTimerId = g_timeout_add_seconds(trg_prefs_get_int(prefs,
                                                                   TRG_PREFS_KEY_SESSION_UPDATE_INTERVAL,
                                                                   TRG_PREFS_CONNECTION),
                                                 trg_session_update_timerfunc,
                                                 win);

    return FALSE;
}

static gboolean on_session_get(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;

    TrgClient *client = priv->client;
    gboolean isConnected = trg_client_is_connected(client);
    JsonObject *lastSession = trg_client_get_session(client);
    JsonObject *newSession = NULL;

    if (response->obj)
        newSession = get_arguments(response->obj);

    if (!isConnected) {
        gdouble version;

        if (trg_dialog_error_handler(win, response)) {
            trg_response_free(response);
            reset_connect_args(win);
            return FALSE;
        }

        if ((version =
             session_get_version(newSession)) < TRANSMISSION_MIN_SUPPORTED)
        {
            gchar *msg =
                g_strdup_printf(_
                                ("This application supports Transmission %g and later, you have %g."),
TRANSMISSION_MIN_SUPPORTED, version);
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win),
                                                       GTK_DIALOG_MODAL,
                                                       GTK_MESSAGE_ERROR,
                                                       GTK_BUTTONS_OK,
                                                       "%s",
                                                       msg);
            gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            g_free(msg);
            trg_response_free(response);
            reset_connect_args(win);
            return FALSE;
        }

        trg_status_bar_connect(priv->statusBar, newSession, client);
    }

    if (newSession) {
        gboolean reloadAliases = lastSession
            && g_strcmp0(session_get_download_dir(lastSession),
                         session_get_download_dir(newSession));
        gboolean refreshSpeed = lastSession
            &&
            (hasEnabledChanged
             (lastSession, newSession, SGET_ALT_SPEED_ENABLED)
             || hasEnabledChanged(lastSession, newSession,
                                  SGET_SPEED_LIMIT_DOWN_ENABLED)
             || hasEnabledChanged(lastSession, newSession,
                                  SGET_SPEED_LIMIT_UP_ENABLED));

        trg_client_set_session(client, newSession);

        if (reloadAliases)
            trg_main_window_reload_dir_aliases(win);

        if (refreshSpeed)
            trg_status_bar_update_speed(priv->statusBar,
                                        trg_torrent_model_get_stats
                                        (priv->torrentModel),
                                        priv->client);
    }

    if (!isConnected) {
        trg_main_window_conn_changed(win, TRUE);
        trg_trackers_tree_view_new_connection(priv->trackersTreeView,
                                              client);
        dispatch_async(client, torrent_get(TORRENT_GET_TAG_MODE_FULL),
                       on_torrent_get_first, win);
    }

    trg_response_free(response);

    return FALSE;
}

static void
connchange_whatever_statusicon(TrgMainWindow * win, gboolean connected)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    gchar *display = connected ?
        trg_prefs_get_string(prefs, TRG_PREFS_KEY_PROFILE_NAME,
                             TRG_PREFS_CONNECTION) :
        g_strdup(_("Disconnected"));

#ifdef HAVE_LIBAPPINDICATOR
    if (priv->appIndicator) {
        GtkMenu *menu = trg_status_icon_view_menu(win, display);
        app_indicator_set_menu(priv->appIndicator, menu);
    } else {
#else
    if (1) {
#endif
        if (priv->iconMenu)
            gtk_widget_destroy(GTK_WIDGET(priv->iconMenu));

        priv->iconMenu = trg_status_icon_view_menu(win, display);

        if (priv->statusIcon)
            gtk_status_icon_set_tooltip_text(priv->statusIcon, display);
    }

    g_free(display);
}

static void
update_whatever_statusicon(TrgMainWindow * win,
                           trg_torrent_model_update_stats * stats)
{
    TrgMainWindowPrivate *priv = win->priv;

#ifdef HAVE_LIBAPPINDICATOR
    if (!priv->appIndicator && !priv->statusIcon)
#else
    if (!priv->statusIcon)
#endif
        return;

    gtk_widget_set_visible(priv->iconSeedingItem, stats != NULL);
    gtk_widget_set_visible(priv->iconDownloadingItem, stats != NULL);
    gtk_widget_set_visible(priv->iconSepItem, stats != NULL);

    if (stats) {
        gchar *downloadingLabel;
        gchar *seedingLabel;
        gchar buf[32];

        trg_strlspeed(buf, stats->downRateTotal / disk_K);
        downloadingLabel = g_strdup_printf(_("%d Downloading @ %s"),
                                           stats->down, buf);
        gtk_menu_item_set_label(GTK_MENU_ITEM(priv->iconDownloadingItem),
                                downloadingLabel);
        g_free(downloadingLabel);

        trg_strlspeed(buf, stats->upRateTotal / disk_K);
        seedingLabel = g_strdup_printf(_("%d Seeding @ %s"),
                                       stats->seeding, buf);
        gtk_menu_item_set_label(GTK_MENU_ITEM(priv->iconSeedingItem),
                                seedingLabel);
        g_free(seedingLabel);
    }
}

/*
 * The callback for a torrent-get response.
 */

static gboolean on_torrent_get(gpointer data, int mode)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *client = priv->client;
    TrgPrefs *prefs = trg_client_get_prefs(client);
    trg_torrent_model_update_stats *stats;
    guint interval;
    gint old_sort_id;
    GtkSortType old_order;

    /* Disconnected between request and response callback */
    if (!trg_client_is_connected(client)) {
        trg_response_free(response);
        return FALSE;
    }

    interval =
        gtk_widget_get_visible(GTK_WIDGET(win)) ? trg_prefs_get_int(prefs,
                                                                    TRG_PREFS_KEY_UPDATE_INTERVAL,
                                                                    TRG_PREFS_CONNECTION)
        : trg_prefs_get_int(prefs, TRG_PREFS_KEY_MINUPDATE_INTERVAL,
                            TRG_PREFS_CONNECTION);
    if (interval < 1)
        interval = TRG_INTERVAL_DEFAULT;

    if (response->status != CURLE_OK) {
        gint64 max_retries =
            trg_prefs_get_int(prefs, TRG_PREFS_KEY_RETRIES,
                              TRG_PREFS_CONNECTION);

        if (trg_client_inc_failcount(client) >= max_retries) {
            trg_main_window_conn_changed(win, FALSE);
            trg_dialog_error_handler(win, response);
        } else {
            gchar *msg =
                make_error_message(response->obj, response->status);
            gchar *statusBarMsg =
                g_strdup_printf(_("Request %d/%d failed: %s"),
                                trg_client_get_failcount(client),
                                (gint) max_retries, msg);
            trg_status_bar_push_connection_msg(priv->statusBar,
                                               statusBarMsg);
            g_free(msg);
            g_free(statusBarMsg);
            priv->timerId = g_timeout_add_seconds(interval,
                                                  trg_update_torrents_timerfunc,
                                                  win);
        }

        trg_response_free(response);

        return FALSE;
    }

    trg_client_reset_failcount(client);
    trg_client_inc_serial(client);

    if (mode != TORRENT_GET_MODE_FIRST)
        gtk_widget_freeze_child_notify(GTK_WIDGET(priv->torrentTreeView));

    gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE
                                         (priv->sortedTorrentModel),
                                         &old_sort_id, &old_order);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE
                                         (priv->sortedTorrentModel),
                                         GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                         GTK_SORT_ASCENDING);

    stats =
        trg_torrent_model_update(priv->torrentModel, client, response->obj,
                                 mode);

    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE
                                         (priv->sortedTorrentModel),
                                         old_sort_id, old_order);

    if (mode != TORRENT_GET_MODE_FIRST)
        gtk_widget_thaw_child_notify(GTK_WIDGET(priv->torrentTreeView));

    update_selected_torrent_notebook(win, mode, priv->selectedTorrentId);
    trg_status_bar_update(priv->statusBar, stats, client);
    update_whatever_statusicon(win, stats);

#if TRG_WITH_GRAPH
    if (priv->graphNotebookIndex >= 0)
        trg_torrent_graph_set_speed(priv->graph, stats);
#endif

    if (mode != TORRENT_GET_MODE_INTERACTION)
        priv->timerId = g_timeout_add_seconds(interval,
                                              trg_update_torrents_timerfunc,
                                              win);

    trg_response_free(response);
    return FALSE;
}

static gboolean on_torrent_get_active(gpointer data)
{
    return on_torrent_get(data, TORRENT_GET_MODE_ACTIVE);
}

static gboolean on_torrent_get_first(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;

    gboolean result = on_torrent_get(data, TORRENT_GET_MODE_FIRST);

    if (priv->args) {
        trg_add_from_filename(win, priv->args);
        priv->args = NULL;
    }

    return result;
}

static gboolean on_torrent_get_interactive(gpointer data)
{
    return on_torrent_get(data, TORRENT_GET_MODE_INTERACTION);
}

static gboolean on_torrent_get_update(gpointer data)
{
    return on_torrent_get(data, TORRENT_GET_MODE_UPDATE);
}

static gboolean trg_session_update_timerfunc(gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;

    trg_client_update_session(priv->client, on_session_get_timer, win);

    return FALSE;
}

static gboolean trg_update_torrents_timerfunc(gpointer data)
{
    /* Check if the TrgMainWindow* has already been destroyed
     * and, in that case, stop polling the server. */
    if (!TRG_IS_MAIN_WINDOW (data))
      return FALSE;

    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *tc = priv->client;
    TrgPrefs *prefs = trg_client_get_prefs(tc);

    if (trg_client_is_connected(tc)) {
        gboolean activeOnly = trg_prefs_get_bool(prefs,
                                                 TRG_PREFS_KEY_UPDATE_ACTIVE_ONLY,
                                                 TRG_PREFS_CONNECTION)
            && (!trg_prefs_get_bool(prefs,
                                    TRG_PREFS_ACTIVEONLY_FULLSYNC_ENABLED,
                                    TRG_PREFS_CONNECTION)
                || (trg_client_get_serial(tc) % trg_prefs_get_int(prefs,
                                                                  TRG_PREFS_ACTIVEONLY_FULLSYNC_EVERY,
                                                                  TRG_PREFS_CONNECTION)
                    != 0));
        dispatch_async(tc,
                       torrent_get(activeOnly ? TORRENT_GET_TAG_MODE_UPDATE
                                   : TORRENT_GET_TAG_MODE_FULL),
                       activeOnly ? on_torrent_get_active :
                       on_torrent_get_update, data);
    }

    return FALSE;
}

static void open_about_cb(GtkWidget * w G_GNUC_UNUSED, GtkWindow * parent)
{
    GtkWidget *aboutDialog = trg_about_window_new(parent);

    gtk_dialog_run(GTK_DIALOG(aboutDialog));
    gtk_widget_destroy(aboutDialog);
}

static gboolean
trg_torrent_tree_view_visible_func(GtkTreeModel * model,
                                   GtkTreeIter * iter, gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    TrgMainWindowPrivate *priv = win->priv;
    guint flags;
    gboolean visible;
    const gchar *filterText;

    guint32 criteria = trg_state_selector_get_flag(priv->stateSelector);

    gtk_tree_model_get(model, iter, TORRENT_COLUMN_FLAGS, &flags, -1);

    if (criteria != 0) {
        if (criteria & FILTER_FLAG_TRACKER) {
            gchar *text =
                trg_state_selector_get_selected_text(priv->stateSelector);
            JsonObject *json = NULL;
            gboolean matchesTracker;
            gtk_tree_model_get(model, iter, TORRENT_COLUMN_JSON, &json,
                               -1);
            matchesTracker = (!json
                              || !torrent_has_tracker(json,
                                                      trg_state_selector_get_url_host_regex
                                                      (priv->
                                                       stateSelector),
                                                      text));
            g_free(text);
            if (matchesTracker)
                return FALSE;
        } else if (criteria & FILTER_FLAG_DIR) {
            gchar *text =
                trg_state_selector_get_selected_text(priv->stateSelector);
            gchar *dd;
            int cmp;
            gtk_tree_model_get(model, iter,
                               TORRENT_COLUMN_DOWNLOADDIR_SHORT, &dd, -1);
            cmp = g_strcmp0(text, dd);
            g_free(dd);
            g_free(text);
            if (cmp)
                return FALSE;
        } else if (!(flags & criteria)) {
            return FALSE;
        }
    }

    visible = TRUE;

    filterText = gtk_entry_get_text(GTK_ENTRY(priv->filterEntry));
    if (strlen(filterText) > 0) {
        gchar *name = NULL;
        gtk_tree_model_get(model, iter, TORRENT_COLUMN_NAME, &name, -1);
        if (name) {
            gchar *filterCmp = g_utf8_casefold(filterText, -1);
            gchar *nameCmp = g_utf8_casefold(name, -1);

            if (!strstr(nameCmp, filterCmp))
                visible = FALSE;

            g_free(nameCmp);
            g_free(filterCmp);
            g_free(name);
        }
    }

    return visible;
}

void trg_main_window_reload_dir_aliases(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    trg_torrent_model_reload_dir_aliases(priv->client, GTK_TREE_MODEL
                                         (priv->torrentModel));
}

static TrgTorrentTreeView
    * trg_main_window_torrent_tree_view_new(TrgMainWindow * win,
                                            GtkTreeModel * model)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgTorrentTreeView *torrentTreeView =
        trg_torrent_tree_view_new(priv->client,
                                  model);

    GtkTreeSelection *selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(torrentTreeView));

    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(torrent_selection_changed), win);

    return torrentTreeView;
}

static gboolean
trg_dialog_error_handler(TrgMainWindow * win, trg_response * response)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (response->status != CURLE_OK) {
        GtkWidget *dialog;
        const gchar *msg;

        msg = make_error_message(response->obj, response->status);
        trg_status_bar_clear_indicators(priv->statusBar);
        trg_status_bar_push_connection_msg(priv->statusBar, msg);
        dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                        "%s", msg);
        gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free((gpointer) msg);
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean
torrent_selection_changed(GtkTreeSelection * selection,
                          TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    GList *selectionList;
    GList *firstNode;
    gint64 id;

    if (trg_torrent_model_is_remove_in_progress(priv->torrentModel)) {
        trg_main_window_torrent_scrub(win);
        return TRUE;
    }

    selectionList = gtk_tree_selection_get_selected_rows(selection, NULL);
    firstNode = g_list_first(selectionList);
    id = -1;

    if (firstNode) {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter(priv->filteredTorrentModel, &iter,
                                    (GtkTreePath *) firstNode->data)) {
            gtk_tree_model_get(priv->filteredTorrentModel, &iter,
                               TORRENT_COLUMN_ID, &id, -1);
        }
    }

    g_list_foreach(selectionList, (GFunc) gtk_tree_path_free, NULL);
    g_list_free(selectionList);

    update_selected_torrent_notebook(win, TORRENT_GET_MODE_FIRST, id);

    return TRUE;
}

gboolean on_delete_complete(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *tc = priv->client;

    if (trg_client_is_connected(tc) && response->status == CURLE_OK)
        trg_client_update_session(priv->client, on_session_get,
                                  response->cb_data);

    return on_generic_interactive_action(data);
}

gboolean on_generic_interactive_action(gpointer data)
{
    trg_response *response = (trg_response *) data;
    TrgMainWindow *win = TRG_MAIN_WINDOW(response->cb_data);
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *tc = priv->client;

    if (trg_client_is_connected(tc)) {
        trg_dialog_error_handler(win, response);

        if (response->status == CURLE_OK) {
            gint64 id;
            if (json_object_has_member(response->obj, PARAM_TAG))
                id = json_object_get_int_member(response->obj, PARAM_TAG);
            else
                id = TORRENT_GET_TAG_MODE_FULL;

            dispatch_async(tc, torrent_get(id), on_torrent_get_interactive,
                           win);
        }
    }

    trg_response_free(response);
    return FALSE;
}

static void trg_main_window_torrent_scrub(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    gtk_tree_store_clear(GTK_TREE_STORE(priv->filesModel));
    gtk_list_store_clear(GTK_LIST_STORE(priv->trackersModel));
    gtk_list_store_clear(GTK_LIST_STORE(priv->peersModel));
    trg_general_panel_clear(priv->genDetails);
    trg_trackers_model_set_no_selection(TRG_TRACKERS_MODEL
                                        (priv->trackersModel));

    trg_toolbar_torrent_actions_sensitive(priv->toolBar, FALSE);
    trg_menu_bar_torrent_actions_sensitive(priv->menuBar, FALSE);
}

static void entry_filter_changed_cb(GtkWidget * w, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    gboolean clearSensitive = gtk_entry_get_text_length(GTK_ENTRY(w)) > 0;

    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER
                                   (priv->filteredTorrentModel));

    g_object_set(priv->filterEntry, "secondary-icon-sensitive",
                 clearSensitive, NULL);
}

static void
torrent_state_selection_changed(TrgStateSelector *
                                selector G_GNUC_UNUSED,
                                guint flag G_GNUC_UNUSED, gpointer data)
{
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(data));
}

static void
trg_main_window_conn_changed(TrgMainWindow * win, gboolean connected)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *tc = priv->client;

    trg_toolbar_connected_change(priv->toolBar, connected);
    trg_menu_bar_connected_change(priv->menuBar, connected);

    gtk_widget_set_sensitive(GTK_WIDGET(priv->torrentTreeView), connected);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->peersTreeView), connected);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->filesTreeView), connected);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->trackersTreeView),
                             connected);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->genDetails), connected);

    if (connected) {
        TrgPrefs *prefs = trg_client_get_prefs(priv->client);
        priv->sessionTimerId =
            g_timeout_add_seconds(trg_prefs_get_int
                                  (prefs,
                                   TRG_PREFS_KEY_SESSION_UPDATE_INTERVAL,
                                   TRG_PREFS_CONNECTION),
                                  trg_session_update_timerfunc, win);
    } else {
        trg_main_window_torrent_scrub(win);
        trg_state_selector_disconnect(priv->stateSelector);

#if TRG_WITH_GRAPH
        if (priv->graphNotebookIndex >= 0)
            trg_torrent_graph_set_nothing(priv->graph);
#endif

        trg_torrent_model_remove_all(priv->torrentModel);

        g_source_remove(priv->timerId);
        g_source_remove(priv->sessionTimerId);
        priv->sessionTimerId = priv->timerId = 0;
    }

    trg_client_status_change(tc, connected);
    connchange_whatever_statusicon(win, connected);
}

static void
trg_main_window_get_property(GObject * object,
                             guint property_id, GValue * value,
                             GParamSpec * pspec)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(object);
    TrgMainWindowPrivate *priv = win->priv;

    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    case PROP_MINIMISE_ON_START:
        g_value_set_boolean(value, priv->min_on_start);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_main_window_set_property(GObject * object,
                             guint property_id,
                             const GValue * value, GParamSpec * pspec)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(object);
    TrgMainWindowPrivate *priv = win->priv;

    switch (property_id) {
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    case PROP_MINIMISE_ON_START:
        priv->min_on_start = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void quit_cb(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}

static TrgMenuBar *trg_main_window_menu_bar_new(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    GObject *b_disconnect, *b_add, *b_resume, *b_pause, *b_verify,
        *b_remove, *b_delete, *b_props, *b_local_prefs, *b_remote_prefs,
        *b_about, *b_view_states, *b_view_notebook, *b_view_stats,
        *b_add_url, *b_quit, *b_move, *b_reannounce, *b_pause_all,
        *b_resume_all, *b_dir_filters, *b_tracker_filters, *b_up_queue,
        *b_down_queue, *b_top_queue, *b_bottom_queue,
#if TRG_WITH_GRAPH
    *b_show_graph,
#endif
    *b_start_now;

    TrgMenuBar *menuBar;
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new();

    menuBar =
        trg_menu_bar_new(win, trg_client_get_prefs(priv->client),
                         priv->torrentTreeView, accel_group);

    g_object_get(menuBar, "disconnect-button", &b_disconnect, "add-button",
                 &b_add, "add-url-button", &b_add_url, "resume-button",
                 &b_resume, "resume-all-button", &b_resume_all,
                 "pause-button", &b_pause, "pause-all-button",
                 &b_pause_all, "delete-button", &b_delete, "remove-button",
                 &b_remove, "move-button", &b_move, "verify-button",
                 &b_verify, "reannounce-button", &b_reannounce,
                 "props-button", &b_props, "remote-prefs-button",
                 &b_remote_prefs, "local-prefs-button", &b_local_prefs,
                 "view-notebook-button", &b_view_notebook,
                 "view-states-button", &b_view_states, "view-stats-button",
                 &b_view_stats, "about-button", &b_about, "quit-button",
                 &b_quit, "dir-filters", &b_dir_filters, "tracker-filters",
                 &b_tracker_filters,
#if TRG_WITH_GRAPH
                 "show-graph", &b_show_graph,
#endif
                 "up-queue", &b_up_queue, "down-queue", &b_down_queue,
                 "top-queue", &b_top_queue, "bottom-queue",
                 &b_bottom_queue, "start-now", &b_start_now, NULL);

    g_signal_connect(b_disconnect, "activate", G_CALLBACK(disconnect_cb),
                     win);
    g_signal_connect(b_add, "activate", G_CALLBACK(add_cb), win);
    g_signal_connect(b_add_url, "activate", G_CALLBACK(add_url_cb), win);
    g_signal_connect(b_resume, "activate", G_CALLBACK(resume_cb), win);
    g_signal_connect(b_resume_all, "activate", G_CALLBACK(resume_all_cb),
                     win);
    g_signal_connect(b_pause, "activate", G_CALLBACK(pause_cb), win);
    g_signal_connect(b_pause_all, "activate", G_CALLBACK(pause_all_cb),
                     win);
    g_signal_connect(b_verify, "activate", G_CALLBACK(verify_cb), win);
    g_signal_connect(b_reannounce, "activate", G_CALLBACK(reannounce_cb),
                     win);
    g_signal_connect(b_delete, "activate", G_CALLBACK(delete_cb), win);
    g_signal_connect(b_remove, "activate", G_CALLBACK(remove_cb), win);
    g_signal_connect(b_up_queue, "activate", G_CALLBACK(up_queue_cb), win);
    g_signal_connect(b_down_queue, "activate", G_CALLBACK(down_queue_cb),
                     win);
    g_signal_connect(b_top_queue, "activate", G_CALLBACK(top_queue_cb),
                     win);
    g_signal_connect(b_bottom_queue, "activate",
                     G_CALLBACK(bottom_queue_cb), win);
    g_signal_connect(b_start_now, "activate", G_CALLBACK(start_now_cb),
                     win);
    g_signal_connect(b_move, "activate", G_CALLBACK(move_cb), win);
    g_signal_connect(b_about, "activate", G_CALLBACK(open_about_cb), win);
    g_signal_connect(b_local_prefs, "activate",
                     G_CALLBACK(open_local_prefs_cb), win);
    g_signal_connect(b_remote_prefs, "activate",
                     G_CALLBACK(open_remote_prefs_cb), win);
    g_signal_connect(b_view_notebook, "toggled",
                     G_CALLBACK(view_notebook_toggled_cb), win);
    g_signal_connect(b_dir_filters, "toggled",
                     G_CALLBACK(main_window_toggle_filter_dirs), win);
    g_signal_connect(b_tracker_filters, "toggled",
                     G_CALLBACK(main_window_toggle_filter_trackers), win);
    g_signal_connect(b_view_states, "toggled",
                     G_CALLBACK(view_states_toggled_cb), win);
    g_signal_connect(b_view_stats, "activate",
                     G_CALLBACK(view_stats_toggled_cb), win);
#if TRG_WITH_GRAPH
    g_signal_connect(b_show_graph, "toggled",
                     G_CALLBACK(trg_main_window_toggle_graph_cb), win);
#endif
    g_signal_connect(b_props, "activate", G_CALLBACK(open_props_cb), win);
    g_signal_connect(b_quit, "activate", G_CALLBACK(quit_cb), win);

    gtk_window_add_accel_group(GTK_WINDOW(win), accel_group);

    return menuBar;
}

static void
status_icon_activated(GtkStatusIcon * icon G_GNUC_UNUSED,
                      TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    trg_main_window_set_hidden_to_tray(win,
                                       !priv->hidden
                                       && trg_prefs_get_bool(prefs,
                                                             TRG_PREFS_KEY_SYSTEM_TRAY_MINIMISE,
                                                             TRG_PREFS_GLOBAL));
}

static gboolean
trg_status_icon_popup_menu_cb(GtkStatusIcon * icon, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    gtk_menu_popup(priv->iconMenu, NULL, NULL,
#ifdef WIN32
                   NULL,
#else
                   gtk_status_icon_position_menu,
#endif
                   priv->statusIcon, 0, gtk_get_current_event_time());

    return TRUE;
}

static gboolean
status_icon_button_press_event(GtkStatusIcon * icon,
                               GdkEventButton * event, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {

        gtk_menu_popup(priv->iconMenu, NULL, NULL,
#ifdef WIN32
                       NULL,
#else
                       gtk_status_icon_position_menu,
#endif
                       priv->statusIcon,
                       event->button,
                       gdk_event_get_time((GdkEvent *) event));
        return TRUE;
    } else {
        return FALSE;
    }
}

static void
clear_filter_entry_cb(GtkEntry * entry,
                      GtkEntryIconPosition icon_pos,
                      GdkEvent * event, gpointer user_data)
{
    gtk_entry_set_text(entry, "");
}

static GtkWidget *trg_imagemenuitem_new(GtkMenuShell * shell,
                                        const gchar * text, char *stock_id,
                                        gboolean sensitive, GCallback cb,
                                        gpointer cbdata)
{
    GtkWidget *item = gtk_image_menu_item_new_with_label(stock_id);

    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(item), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
                                              (item), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(item), text);
    g_signal_connect(item, "activate", cb, cbdata);
    gtk_widget_set_sensitive(item, sensitive);
    gtk_menu_shell_append(shell, item);

    return item;
}

static void set_limit_cb(GtkWidget * w, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    GtkWidget *parent = gtk_widget_get_parent(w);

    gint speed = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "limit"));
    gchar *speedKey = g_object_get_data(G_OBJECT(parent), "speedKey");
    gchar *enabledKey = g_object_get_data(G_OBJECT(parent), "enabledKey");
    gpointer limitIds = g_object_get_data(G_OBJECT(parent), "limit-ids");

    JsonNode *req = NULL;
    JsonObject *args;

    if (limitIds)
        req = torrent_set((JsonArray *) limitIds);
    else
        req = session_set();

    args = node_get_arguments(req);

    if (speed >= 0)
        json_object_set_int_member(args, speedKey, speed);

    json_object_set_boolean_member(args, enabledKey, speed >= 0);

    if (limitIds)
        dispatch_async(priv->client, req, on_generic_interactive_action,
                       win);
    else
        dispatch_async(priv->client, req, on_session_set, win);
}

static void set_priority_cb(GtkWidget * w, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    GtkWidget *parent = gtk_widget_get_parent(w);

    gint priority =
        GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "priority"));
    gpointer limitIds = g_object_get_data(G_OBJECT(parent), "pri-ids");

    JsonNode *req = NULL;
    JsonObject *args;

    req = torrent_set((JsonArray *) limitIds);

    args = node_get_arguments(req);

    json_object_set_int_member(args, FIELD_BANDWIDTH_PRIORITY, priority);

    dispatch_async(priv->client, req, on_generic_interactive_action, win);
}

static GtkWidget *limit_item_new(TrgMainWindow * win, GtkWidget * menu,
                                 gint64 currentLimit, gfloat limit)
{
    char speed[32];
    GtkWidget *item;
    gboolean active = limit < 0 ? FALSE : (currentLimit == (gint64) limit);

    trg_strlspeed(speed, limit);

    item = gtk_check_menu_item_new_with_label(speed);

    g_object_set_data(G_OBJECT(item), "limit",
                      GINT_TO_POINTER((gint) limit));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), active);
    g_signal_connect(item, "activate", G_CALLBACK(set_limit_cb), win);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    return item;
}

static GtkWidget *priority_menu_item_new(TrgMainWindow * win,
                                         GtkMenuShell * menu,
                                         const gchar * label, gint value,
                                         gint current_value)
{
    GtkWidget *item = gtk_check_menu_item_new_with_label(label);

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
                                   value == current_value);
    g_object_set_data(G_OBJECT(item), "priority", GINT_TO_POINTER(value));
    g_signal_connect(item, "activate", G_CALLBACK(set_priority_cb), win);

    gtk_menu_shell_append(menu, item);

    return item;
}

static GtkWidget *priority_menu_new(TrgMainWindow * win, JsonArray * ids)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *client = priv->client;
    JsonObject *t = NULL;
    gint selected_pri = TR_PRI_UNSET;
    GtkWidget *toplevel, *menu;

    if (get_torrent_data(trg_client_get_torrent_table(client),
                         priv->selectedTorrentId, &t, NULL))
        selected_pri = torrent_get_bandwidth_priority(t);

    toplevel = gtk_image_menu_item_new_with_label(GTK_STOCK_NETWORK);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(toplevel), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
                                              (toplevel), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(toplevel), _("Priority"));

    menu = gtk_menu_new();

    g_object_set_data_full(G_OBJECT(menu), "pri-ids", ids,
                           (GDestroyNotify) json_array_unref);

    priority_menu_item_new(win, GTK_MENU_SHELL(menu), _("High"),
                           TR_PRI_HIGH, selected_pri);
    priority_menu_item_new(win, GTK_MENU_SHELL(menu), _("Normal"),
                           TR_PRI_NORMAL, selected_pri);
    priority_menu_item_new(win, GTK_MENU_SHELL(menu), _("Low"), TR_PRI_LOW,
                           selected_pri);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(toplevel), menu);

    return toplevel;
}

static GtkWidget *limit_menu_new(TrgMainWindow * win, gchar * title,
                                 gchar * enabledKey, gchar * speedKey,
                                 JsonArray * ids)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgClient *client = priv->client;
    JsonObject *current = NULL;
    GtkTreeIter iter;
    GtkWidget *toplevel, *menu, *item;
    gint64 limit;

    if (ids)
        get_torrent_data(trg_client_get_torrent_table(client),
                         priv->selectedTorrentId, &current, &iter);
    else
        current = trg_client_get_session(client);

    limit =
        json_object_get_boolean_member(current,
                                       enabledKey) ?
        json_object_get_int_member(current, speedKey) : -1;
    toplevel = gtk_image_menu_item_new_with_label(GTK_STOCK_NETWORK);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(toplevel), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
                                              (toplevel), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(toplevel), title);

    menu = gtk_menu_new();

    g_object_set_data_full(G_OBJECT(menu), "speedKey", g_strdup(speedKey),
                           g_free);
    g_object_set_data_full(G_OBJECT(menu), "enabledKey",
                           g_strdup(enabledKey), g_free);
    g_object_set_data_full(G_OBJECT(menu), "limit-ids", ids,
                           (GDestroyNotify) json_array_unref);

    item = gtk_check_menu_item_new_with_label(_("No Limit"));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), limit < 0);
    g_object_set_data(G_OBJECT(item), "limit", GINT_TO_POINTER(-1));
    g_signal_connect(item, "activate", G_CALLBACK(set_limit_cb), win);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());

    limit_item_new(win, menu, limit, 0);
    limit_item_new(win, menu, limit, 5);
    limit_item_new(win, menu, limit, 10);
    limit_item_new(win, menu, limit, 25);
    limit_item_new(win, menu, limit, 50);
    limit_item_new(win, menu, limit, 75);
    limit_item_new(win, menu, limit, 100);
    limit_item_new(win, menu, limit, 150);
    limit_item_new(win, menu, limit, 200);
    limit_item_new(win, menu, limit, 300);
    limit_item_new(win, menu, limit, 400);
    limit_item_new(win, menu, limit, 500);
    limit_item_new(win, menu, limit, 750);
    limit_item_new(win, menu, limit, 1024);
    limit_item_new(win, menu, limit, 1280);
    limit_item_new(win, menu, limit, 1536);
    limit_item_new(win, menu, limit, 2048);
    limit_item_new(win, menu, limit, 2560);
    limit_item_new(win, menu, limit, 3072);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(toplevel), menu);

    return toplevel;
}

static void exec_cmd_cb(GtkWidget * w, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    JsonObject *cmd_obj = (JsonObject *) g_object_get_data(G_OBJECT(w),
                                                           "cmd-object");
    GtkTreeSelection *selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->torrentTreeView));
    GtkTreeModel *model;
    GList *selectedRows = gtk_tree_selection_get_selected_rows(selection,
                                                               &model);
    GError *cmd_error = NULL;
    gchar *cmd_line = NULL;
    gchar **argv = NULL;

    cmd_line = build_remote_exec_cmd(priv->client,
                                     model,
                                     selectedRows,
                                     json_object_get_string_member(cmd_obj,
                                                                   TRG_PREFS_KEY_EXEC_COMMANDS_SUBKEY_CMD));

    g_debug("Exec: %s", cmd_line);

    if (!cmd_line)
        return;

    /* GTK has bug, won't let you pass a string here containing a quoted param, so use parse and then spawn
     * rather than g_spawn_command_line_async(cmd_line,&cmd_error); */

    g_shell_parse_argv(cmd_line, NULL, &argv, NULL);
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL,
                  &cmd_error);

    g_list_foreach(selectedRows, (GFunc) gtk_tree_path_free, NULL);
    g_list_free(selectedRows);

    if (argv)
        g_strfreev(argv);

    g_free(cmd_line);

    if (cmd_error) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win),
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_OK, "%s",
                                                   cmd_error->message);
        gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_error_free(cmd_error);
    }
}

static void
trg_torrent_tv_view_menu(GtkWidget * treeview,
                         GdkEventButton * event, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    GtkWidget *menu;
    gint n_cmds;
    JsonArray *ids;
    JsonArray *cmds;

    menu = gtk_menu_new();
    ids = build_json_id_array(TRG_TORRENT_TREE_VIEW(treeview));

    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Properties"),
                          GTK_STOCK_PROPERTIES, TRUE,
                          G_CALLBACK(open_props_cb), win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Resume"),
                          GTK_STOCK_MEDIA_PLAY, TRUE,
                          G_CALLBACK(resume_cb), win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Pause"),
                          GTK_STOCK_MEDIA_PAUSE, TRUE,
                          G_CALLBACK(pause_cb), win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Verify"),
                          GTK_STOCK_REFRESH, TRUE, G_CALLBACK(verify_cb),
                          win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Re-announce"),
                          GTK_STOCK_REFRESH, TRUE,
                          G_CALLBACK(reannounce_cb), win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Move"),
                          GTK_STOCK_HARDDISK, TRUE, G_CALLBACK(move_cb),
                          win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Remove"),
                          GTK_STOCK_REMOVE, TRUE, G_CALLBACK(remove_cb),
                          win);
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Remove & Delete"),
                          GTK_STOCK_CLEAR, TRUE, G_CALLBACK(delete_cb),
                          win);

    cmds = trg_prefs_get_array(prefs, TRG_PREFS_KEY_EXEC_COMMANDS,
                               TRG_PREFS_CONNECTION);
    n_cmds = json_array_get_length(cmds);

    if (n_cmds > 0) {
        GList *cmds_list = json_array_get_elements(cmds);
        GtkMenuShell *cmds_shell;
        GList *cmds_li;

        if (n_cmds < 3) {
            gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                                  gtk_separator_menu_item_new());
            cmds_shell = GTK_MENU_SHELL(menu);
        } else {
            GtkImageMenuItem *cmds_menu =
                GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label
                                    (GTK_STOCK_EXECUTE));
            gtk_image_menu_item_set_use_stock(cmds_menu, TRUE);
            gtk_image_menu_item_set_always_show_image(cmds_menu, TRUE);
            gtk_menu_item_set_label(GTK_MENU_ITEM(cmds_menu),
                                    _("Actions"));

            cmds_shell = GTK_MENU_SHELL(gtk_menu_new());
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(cmds_menu),
                                      GTK_WIDGET(cmds_shell));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                                  GTK_WIDGET(cmds_menu));
        }

        for (cmds_li = cmds_list; cmds_li; cmds_li = g_list_next(cmds_li)) {
            JsonObject *cmd_obj = json_node_get_object((JsonNode *)
                                                       cmds_li->data);
            const gchar *cmd_label = json_object_get_string_member(cmd_obj,
                                                                   "label");
            GtkWidget *item = trg_imagemenuitem_new(cmds_shell, cmd_label,
                                                    GTK_STOCK_EXECUTE,
                                                    TRUE,
                                                    G_CALLBACK
                                                    (exec_cmd_cb), win);
            g_object_set_data(G_OBJECT(item), "cmd-object", cmd_obj);
        }

        g_list_free(cmds_list);
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());

    if (priv->queuesEnabled) {
        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Start Now"),
                              GTK_STOCK_MEDIA_PLAY, TRUE,
                              G_CALLBACK(start_now_cb), win);
        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Move Up Queue"),
                              GTK_STOCK_GO_UP, TRUE,
                              G_CALLBACK(up_queue_cb), win);
        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Move Down Queue"),
                              GTK_STOCK_GO_DOWN, TRUE,
                              G_CALLBACK(down_queue_cb), win);
        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Bottom Of Queue"),
                              GTK_STOCK_GOTO_BOTTOM, TRUE,
                              G_CALLBACK(bottom_queue_cb), win);
        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Top Of Queue"),
                              GTK_STOCK_GOTO_TOP, TRUE,
                              G_CALLBACK(top_queue_cb), win);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                              gtk_separator_menu_item_new());
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          limit_menu_new(win,
                                         _("Down Limit"),
                                         FIELD_DOWNLOAD_LIMITED,
                                         FIELD_DOWNLOAD_LIMIT, ids));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          limit_menu_new(win,
                                         _("Up Limit"),
                                         FIELD_UPLOAD_LIMITED,
                                         FIELD_UPLOAD_LIMIT, ids));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          priority_menu_new(win, ids));

    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent *) event));
}

static GtkMenu *trg_status_icon_view_menu(TrgMainWindow * win,
                                          const gchar * msg)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    gboolean connected = trg_client_is_connected(priv->client);
    GtkWidget *menu, *connect;

    menu = gtk_menu_new();

    priv->iconStatusItem = gtk_menu_item_new_with_label(msg);
    gtk_widget_set_sensitive(priv->iconStatusItem, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), priv->iconStatusItem);

    if (connected) {
        priv->iconDownloadingItem =
            gtk_menu_item_new_with_label(_("Updating..."));
        gtk_widget_set_visible(priv->iconDownloadingItem, FALSE);
        gtk_widget_set_sensitive(priv->iconDownloadingItem, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                              priv->iconDownloadingItem);

        priv->iconSeedingItem =
            gtk_menu_item_new_with_label(_("Updating..."));
        gtk_widget_set_visible(priv->iconSeedingItem, FALSE);
        gtk_widget_set_sensitive(priv->iconSeedingItem, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), priv->iconSeedingItem);
    }

    priv->iconSepItem = gtk_separator_menu_item_new();
    gtk_widget_set_sensitive(priv->iconSepItem, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), priv->iconSepItem);

    connect = gtk_image_menu_item_new_with_label(GTK_STOCK_CONNECT);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(connect), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(connect),
                                              TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(connect), _("Connect"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(connect),
                              trg_menu_bar_file_connect_menu_new(win,
                                                                 prefs));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), connect);

    if (connected) {
        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Disconnect"),
                              GTK_STOCK_DISCONNECT, connected,
                              G_CALLBACK(disconnect_cb), win);

        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Add"),
                              GTK_STOCK_ADD, connected, G_CALLBACK(add_cb),
                              win);

        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Add from URL"),
                              GTK_STOCK_ADD, connected,
                              G_CALLBACK(add_url_cb), win);

        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Resume All"),
                              GTK_STOCK_MEDIA_PLAY, connected,
                              G_CALLBACK(resume_all_cb), win);

        trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Pause All"),
                              GTK_STOCK_MEDIA_PAUSE, connected,
                              G_CALLBACK(pause_all_cb), win);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                              limit_menu_new(win, _("Down Limit"),
                                             SGET_SPEED_LIMIT_DOWN_ENABLED,
                                             SGET_SPEED_LIMIT_DOWN, NULL));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                              limit_menu_new(win, _("Up Limit"),
                                             SGET_SPEED_LIMIT_UP_ENABLED,
                                             SGET_SPEED_LIMIT_UP, NULL));
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());
    trg_imagemenuitem_new(GTK_MENU_SHELL(menu), _("Quit"), GTK_STOCK_QUIT,
                          TRUE, G_CALLBACK(quit_cb), win);

    gtk_widget_show_all(menu);

    return GTK_MENU(menu);
}

static gboolean
torrent_tv_button_pressed_cb(GtkWidget * treeview,
                             GdkEventButton * event, gpointer userdata)
{
    GtkTreeSelection *selection;
    GtkTreePath *path;

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                          (gint) event->x, (gint) event->y,
                                          &path, NULL, NULL, NULL)) {
            if (!gtk_tree_selection_path_is_selected(selection, path)) {
                gtk_tree_selection_unselect_all(selection);
                gtk_tree_selection_select_path(selection, path);
            }

            gtk_tree_path_free(path);

            trg_torrent_tv_view_menu(treeview, event, userdata);
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
torrent_tv_popup_menu_cb(GtkWidget * treeview, gpointer userdata)
{
    trg_torrent_tv_view_menu(treeview, NULL, userdata);
    return TRUE;
}

static void trg_main_window_set_hidden_to_tray(TrgMainWindow * win,
                                               gboolean hidden)
{

    TrgMainWindowPrivate *priv = win->priv;

    if (hidden) {
        gtk_widget_hide(GTK_WIDGET(win));
    } else {
        gtk_window_deiconify(GTK_WINDOW(win));
        gtk_window_present(GTK_WINDOW(win));

        if (priv->timerId > 0) {
            g_source_remove(priv->timerId);
            dispatch_async(priv->client,
                           torrent_get(TORRENT_GET_TAG_MODE_FULL),
                           on_torrent_get_update, win);
        }
    }

    priv->hidden = hidden;
}

static gboolean
window_state_event(TrgMainWindow * win,
                   GdkEventWindowState * event, gpointer trayIcon)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    if (priv->statusIcon
        && (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
        && (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)
        && trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SYSTEM_TRAY_MINIMISE,
                              TRG_PREFS_GLOBAL)) {
        trg_main_window_set_hidden_to_tray(win, TRUE);
        return TRUE;
    }

    return FALSE;
}

void trg_main_window_remove_status_icon(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
#ifdef HAVE_LIBAPPINDICATOR
    if (priv->appIndicator) {
        g_object_unref(G_OBJECT(priv->appIndicator));

        priv->appIndicator = NULL;
    } else {
#else
    if (1) {
#endif
        if (priv->statusIcon)
            g_object_unref(G_OBJECT(priv->statusIcon));

        priv->statusIcon = NULL;
    }
}

#if TRG_WITH_GRAPH
void trg_main_window_add_graph(TrgMainWindow * win, gboolean show)
{
    TrgMainWindowPrivate *priv = win->priv;

    priv->graph =
        trg_torrent_graph_new(gtk_widget_get_style(priv->notebook));
    priv->graphNotebookIndex =
        gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook),
                                 GTK_WIDGET(priv->graph),
                                 gtk_label_new(_("Graph")));

    if (show)
        gtk_widget_show_all(priv->notebook);

    trg_torrent_graph_start(priv->graph);
}

void trg_main_window_remove_graph(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;

    if (priv->graphNotebookIndex >= 0) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(priv->notebook),
                                 priv->graphNotebookIndex);
        priv->graphNotebookIndex = -1;
    }
}
#endif

/*static gboolean status_icon_size_changed(GtkStatusIcon *status_icon,
        gint           size,
        gpointer       user_data)
{
    gtk_status_icon_set_from_icon_name(status_icon, PACKAGE_NAME);
    return TRUE;
}*/

void trg_main_window_add_status_icon(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
#ifdef HAVE_LIBAPPINDICATOR
    if (is_unity() && (priv->appIndicator =
                       app_indicator_new(PACKAGE_NAME, PACKAGE_NAME,
                                         APP_INDICATOR_CATEGORY_APPLICATION_STATUS)))
    {
        app_indicator_set_status(priv->appIndicator,
                                 APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_menu(priv->appIndicator,
                               trg_status_icon_view_menu(win, NULL));
    } else {
#else
    if (!is_unity()) {
#endif
        priv->statusIcon =
            gtk_status_icon_new_from_icon_name(PACKAGE_NAME);
        gtk_status_icon_set_screen(priv->statusIcon,
                                   gtk_window_get_screen(GTK_WINDOW(win)));
        g_signal_connect(priv->statusIcon, "activate",
                         G_CALLBACK(status_icon_activated), win);
        g_signal_connect(priv->statusIcon, "button-press-event",
                         G_CALLBACK(status_icon_button_press_event), win);
        g_signal_connect(priv->statusIcon, "popup-menu",
                         G_CALLBACK(trg_status_icon_popup_menu_cb), win);

        gtk_status_icon_set_visible(priv->statusIcon, TRUE);
    }

    connchange_whatever_statusicon(win,
                                   trg_client_is_connected(priv->client));
}

TrgStateSelector *trg_main_window_get_state_selector(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    return priv->stateSelector;
}

/* Couldn't find a way to get the width/height on exit, so save the
 * values of this event for when that happens. */
static gboolean
trg_main_window_config_event(TrgMainWindow * win,
                             GdkEvent * event,
                             gpointer user_data G_GNUC_UNUSED)
{
    TrgMainWindowPrivate *priv = win->priv;
    priv->width = event->configure.width;
    priv->height = event->configure.height;
    return FALSE;
}

static void
trg_client_session_updated_cb(TrgClient * tc,
                              JsonObject * session, TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    gboolean queuesEnabled;

    trg_status_bar_session_update(priv->statusBar, session);

    if (json_object_has_member(session, SGET_DOWNLOAD_QUEUE_ENABLED)) {
        queuesEnabled = json_object_get_boolean_member(session,
                                                       SGET_DOWNLOAD_QUEUE_ENABLED)
            || json_object_get_boolean_member(session,
                                              SGET_SEED_QUEUE_ENABLED);
    } else {
        queuesEnabled = FALSE;
    }

    if (priv->queuesEnabled != queuesEnabled) {
        trg_menu_bar_set_supports_queues(priv->menuBar, queuesEnabled);
        trg_state_selector_set_queues_enabled(priv->stateSelector,
                                              queuesEnabled);
    }

    priv->queuesEnabled = queuesEnabled;
}

/* Drag & Drop support */
static GtkTargetEntry target_list[] = {
/* datatype (string), restrictions on DnD (GtkTargetFlags), datatype (int) */
    {"text/uri-list", GTK_TARGET_OTHER_APP | GTK_TARGET_OTHER_WIDGET, 0}
};

static guint n_targets = G_N_ELEMENTS(target_list);

static void on_dropped_file(GtkWidget * widget, GdkDragContext * context,
                            gint x, gint y, GtkSelectionData * data,
                            guint info, guint time, gpointer user_data)
{
    TrgMainWindow *win = user_data;

    if ((gtk_selection_data_get_length(data) >= 0)
        && (gtk_selection_data_get_format(data) == 8)) {
        gchar **uri_list = gtk_selection_data_get_uris(data);
        guint num_files = g_strv_length(uri_list);
        gchar **file_list = g_new0(gchar *, num_files + 1);
        int i;

        for (i = 0; i < num_files; i++)
            file_list[i] = g_filename_from_uri(uri_list[i], NULL, NULL);

        g_strfreev(uri_list);
        gtk_drag_finish(context, TRUE, FALSE, time);
        trg_add_from_filename(win, file_list);
    } else {
        gtk_drag_finish(context, FALSE, FALSE, time);
    }
}

static gboolean window_key_press_handler(GtkWidget * widget,
                                         GdkEvent * event,
                                         gpointer user_data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(widget);

    if ((event->key.state & GDK_CONTROL_MASK)
        && event->key.keyval == GDK_k) {
        gtk_widget_grab_focus(win->priv->filterEntry);
        return TRUE;
    }

    return FALSE;
}

static GObject *trg_main_window_constructor(GType type,
                                            guint n_construct_properties,
                                            GObjectConstructParam *
                                            construct_params)
{
    TrgMainWindow *self = TRG_MAIN_WINDOW(G_OBJECT_CLASS
                                          (trg_main_window_parent_class)->
                                          constructor(type,
                                                      n_construct_properties,
                                                      construct_params));
    TrgMainWindowPrivate *priv =
        G_TYPE_INSTANCE_GET_PRIVATE(self, TRG_TYPE_MAIN_WINDOW,
                                    TrgMainWindowPrivate);
    GtkWidget *w;
    GtkWidget *outerVbox;
    GtkWidget *toolbarHbox;
    GtkWidget *outerAlignment;
    GtkIconTheme *theme;
    gint width, height, pos;
    gboolean tray;
    TrgPrefs *prefs;

    priv->queuesEnabled = TRUE;

    prefs = trg_client_get_prefs(priv->client);

    theme = gtk_icon_theme_get_default();
    register_my_icons(theme);

#ifdef HAVE_LIBNOTIFY
    notify_init(PACKAGE_NAME);
#endif
    gtk_window_set_default_icon_name(PACKAGE_NAME);

    gtk_window_set_title(GTK_WINDOW(self), _("Transmission Remote"));
    gtk_window_set_default_size(GTK_WINDOW(self), 1000, 600);

    g_signal_connect(G_OBJECT(self), "delete-event",
                     G_CALLBACK(delete_event), NULL);
    g_signal_connect(G_OBJECT(self), "destroy", G_CALLBACK(destroy_window),
                     NULL);
    g_signal_connect(G_OBJECT(self), "window-state-event",
                     G_CALLBACK(window_state_event), NULL);
    g_signal_connect(G_OBJECT(self), "configure-event",
                     G_CALLBACK(trg_main_window_config_event), NULL);
    g_signal_connect(G_OBJECT(self), "key-press-event",
                     G_CALLBACK(window_key_press_handler), NULL);

    priv->torrentModel = trg_torrent_model_new();
    trg_client_set_torrent_table(priv->client,
                                 get_torrent_table(priv->torrentModel));

    g_signal_connect(priv->torrentModel, "torrent-completed",
                     G_CALLBACK(on_torrent_completed), self);
    g_signal_connect(priv->torrentModel, "torrent-added",
                     G_CALLBACK(on_torrent_added), self);

    priv->sortedTorrentModel =
        gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL
                                           (priv->torrentModel));

    priv->filteredTorrentModel =
        trg_sortable_filtered_model_new(GTK_TREE_SORTABLE
                                        (priv->sortedTorrentModel), NULL);
    gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER
                                           (priv->filteredTorrentModel),
                                           trg_torrent_tree_view_visible_func,
                                           self, NULL);

    priv->torrentTreeView = trg_main_window_torrent_tree_view_new(self,
                                                                  priv->
                                                                  filteredTorrentModel);
    g_signal_connect(priv->torrentTreeView, "popup-menu",
                     G_CALLBACK(torrent_tv_popup_menu_cb), self);
    g_signal_connect(priv->torrentTreeView, "button-press-event",
                     G_CALLBACK(torrent_tv_button_pressed_cb), self);
    g_signal_connect(priv->torrentTreeView, "row-activated",
                     G_CALLBACK(torrent_tv_onRowActivated), self);

    outerVbox = trg_vbox_new(FALSE, 6);

    /* Create a GtkAlignment to hold the outerVbox making possible
     * some padding. */
    outerAlignment = gtk_alignment_new (0.5f, 0.5f, 1.0f, 1.0f);
    gtk_alignment_set_padding (GTK_ALIGNMENT (outerAlignment), 0, 0, 6, 6);
    gtk_container_add (GTK_CONTAINER (outerAlignment), outerVbox);

    gtk_container_add(GTK_CONTAINER(self), outerAlignment);

    priv->menuBar = trg_main_window_menu_bar_new(self);
    gtk_box_pack_start(GTK_BOX(outerVbox), GTK_WIDGET(priv->menuBar),
                       FALSE, FALSE, 0);

    toolbarHbox = trg_hbox_new(FALSE, 0);
    priv->toolBar = trg_main_window_toolbar_new(self);
    gtk_box_pack_start(GTK_BOX(toolbarHbox), GTK_WIDGET(priv->toolBar),
                       TRUE, TRUE, 0);

    w = gtk_entry_new();
    gtk_entry_set_icon_from_stock(GTK_ENTRY(w), GTK_ENTRY_ICON_SECONDARY,
                                  GTK_STOCK_CLEAR);
    g_signal_connect(w, "icon-release", G_CALLBACK(clear_filter_entry_cb),
                     NULL);
    gtk_box_pack_start(GTK_BOX(toolbarHbox), w, FALSE, FALSE, 0);
    g_object_set(w, "secondary-icon-sensitive", FALSE, NULL);
    priv->filterEntry = w;

    g_signal_connect(G_OBJECT(priv->filterEntry), "changed",
                     G_CALLBACK(entry_filter_changed_cb), self);

    gtk_box_pack_start(GTK_BOX(outerVbox), GTK_WIDGET(toolbarHbox), FALSE,
                       FALSE, 0);

#if GTK_CHECK_VERSION( 3, 0, 0 )
    priv->hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    priv->vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
#else
    priv->vpaned = gtk_vpaned_new();
    priv->hpaned = gtk_hpaned_new();
#endif

    gtk_box_pack_start(GTK_BOX(outerVbox), priv->vpaned, TRUE, TRUE, 0);
    gtk_paned_pack1(GTK_PANED(priv->vpaned), priv->hpaned, TRUE, TRUE);

    priv->stateSelector = trg_state_selector_new(priv->client,
                                                 priv->torrentModel);
    priv->stateSelectorScroller =
        my_scrolledwin_new(GTK_WIDGET(priv->stateSelector));
    gtk_paned_pack1(GTK_PANED(priv->hpaned), priv->stateSelectorScroller,
                    FALSE, FALSE);

    gtk_paned_pack2(GTK_PANED(priv->hpaned), my_scrolledwin_new(GTK_WIDGET
                                                                (priv->
                                                                 torrentTreeView)),
                    TRUE, TRUE);

    g_signal_connect(G_OBJECT(priv->stateSelector),
                     "torrent-state-changed",
                     G_CALLBACK(torrent_state_selection_changed),
                     priv->filteredTorrentModel);

    priv->notebook = trg_main_window_notebook_new(self);
    gtk_paned_pack2(GTK_PANED(priv->vpaned), priv->notebook, FALSE, FALSE);

    tray = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SYSTEM_TRAY,
                              TRG_PREFS_GLOBAL);
    if (tray)
        trg_main_window_add_status_icon(self);
    else
        trg_main_window_remove_status_icon(self);

    priv->statusBar = trg_status_bar_new(self, priv->client);
    g_signal_connect(priv->client, "session-updated",
                     G_CALLBACK(trg_client_session_updated_cb), self);

    gtk_box_pack_start(GTK_BOX(outerVbox), GTK_WIDGET(priv->statusBar),
                       FALSE, FALSE, 2);

    width = trg_prefs_get_int(prefs, TRG_PREFS_KEY_WINDOW_WIDTH,
                              TRG_PREFS_GLOBAL);
    height = trg_prefs_get_int(prefs, TRG_PREFS_KEY_WINDOW_HEIGHT,
                               TRG_PREFS_GLOBAL);

    pos = trg_prefs_get_int(prefs, TRG_PREFS_KEY_NOTEBOOK_PANED_POS,
                            TRG_PREFS_GLOBAL);

    if (width > 0 && height > 0)
        gtk_window_set_default_size(GTK_WINDOW(self), width, height);
    else if (pos < 1)
        gtk_paned_set_position(GTK_PANED(priv->vpaned), 300);

    if (pos > 0)
        gtk_paned_set_position(GTK_PANED(priv->vpaned), pos);

    gtk_widget_show_all(GTK_WIDGET(self));

    trg_widget_set_visible(priv->stateSelectorScroller,
                           trg_prefs_get_bool(prefs,
                                              TRG_PREFS_KEY_SHOW_STATE_SELECTOR,
                                              TRG_PREFS_GLOBAL));
    trg_widget_set_visible(priv->notebook,
                           trg_prefs_get_bool(prefs,
                                              TRG_PREFS_KEY_SHOW_NOTEBOOK,
                                              TRG_PREFS_GLOBAL));

    pos = trg_prefs_get_int(prefs, TRG_PREFS_KEY_STATES_PANED_POS,
                            TRG_PREFS_GLOBAL);
    if (pos > 0)
        gtk_paned_set_position(GTK_PANED(priv->hpaned), pos);

    if (tray && priv->min_on_start)
        trg_main_window_set_hidden_to_tray(self, TRUE);

    /* Drag and Drop */
    gtk_drag_dest_set(GTK_WIDGET(self), /* widget that will accept a drop */
                      GTK_DEST_DEFAULT_ALL,     /* default actions for dest on DnD */
                      target_list,      /* lists of target to support */
                      n_targets,        /* size of list */
                      GDK_ACTION_MOVE   /* what to do with data after dropped */
                      /* | GDK_ACTION_COPY ... seems that file managers only need ACTION_MOVE, not ACTION_COPY */
        );

    g_signal_connect(self, "drag-data-received",
                     G_CALLBACK(on_dropped_file), self);

    return G_OBJECT(self);
}

static void trg_main_window_class_init(TrgMainWindowClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgMainWindowPrivate));

    object_class->constructor = trg_main_window_constructor;
    object_class->get_property = trg_main_window_get_property;
    object_class->set_property = trg_main_window_set_property;

    g_object_class_install_property(object_class,
                                    PROP_CLIENT,
                                    g_param_spec_pointer("trg-client",
                                                         "TClient",
                                                         "Client",
                                                         G_PARAM_READWRITE
                                                         |
                                                         G_PARAM_CONSTRUCT_ONLY
                                                         |
                                                         G_PARAM_STATIC_NAME
                                                         |
                                                         G_PARAM_STATIC_NICK
                                                         |
                                                         G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_MINIMISE_ON_START,
                                    g_param_spec_boolean("min-on-start",
                                                         "Min On Start",
                                                         "Min On Start",
                                                         FALSE,
                                                         G_PARAM_READWRITE
                                                         |
                                                         G_PARAM_CONSTRUCT_ONLY
                                                         |
                                                         G_PARAM_STATIC_NAME
                                                         |
                                                         G_PARAM_STATIC_NICK
                                                         |
                                                         G_PARAM_STATIC_BLURB));
}

void trg_main_window_set_start_args(TrgMainWindow * win, gchar ** args)
{
    TrgMainWindowPrivate *priv = win->priv;
    priv->args = args;
}

void auto_connect_if_required(TrgMainWindow * win)
{
    TrgMainWindowPrivate *priv = win->priv;
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    gchar *host = trg_prefs_get_string(prefs, TRG_PREFS_KEY_HOSTNAME,
                                       TRG_PREFS_PROFILE);

    if (host) {
        gint len = strlen(host);
        g_free(host);
        if (len > 0
            && trg_prefs_get_bool(prefs, TRG_PREFS_KEY_AUTO_CONNECT,
                                  TRG_PREFS_PROFILE)) {
            connect_cb(NULL, win);
        }
    }
}

TrgMainWindow *trg_main_window_new(TrgClient * tc, gboolean minonstart)
{
    return g_object_new(TRG_TYPE_MAIN_WINDOW, "trg-client", tc,
                        "min-on-start", minonstart, NULL);
}

