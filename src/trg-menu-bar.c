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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION( 3, 0, 0 )
#include <gdk/gdkkeysyms-compat.h>
#endif

#include "trg-prefs.h"
#include "trg-torrent-graph.h"
#include "trg-tree-view.h"
#include "trg-torrent-tree-view.h"
#include "trg-main-window.h"
#include "trg-menu-bar.h"

enum {
    PROP_0,
    PROP_CONNECT_BUTTON,
    PROP_DISCONNECT_BUTTON,
    PROP_ADD_BUTTON,
    PROP_ADD_URL_BUTTON,
    PROP_REMOVE_BUTTON,
    PROP_DELETE_BUTTON,
    PROP_RESUME_BUTTON,
    PROP_RESUME_ALL_BUTTON,
    PROP_PAUSE_BUTTON,
    PROP_PAUSE_ALL_BUTTON,
    PROP_VERIFY_BUTTON,
    PROP_REANNOUNCE_BUTTON,
    PROP_PROPS_BUTTON,
    PROP_MOVE_BUTTON,
    PROP_REMOTE_PREFS_BUTTON,
    PROP_LOCAL_PREFS_BUTTON,
    PROP_ABOUT_BUTTON,
    PROP_VIEW_STATS_BUTTON,
    PROP_VIEW_STATES_BUTTON,
    PROP_VIEW_NOTEBOOK_BUTTON,
    PROP_QUIT,
    PROP_PREFS,
    PROP_MAIN_WINDOW,
    PROP_TORRENT_TREE_VIEW,
    PROP_ACCEL_GROUP,
    PROP_DIR_FILTERS,
    PROP_TRACKER_FILTERS,
#if TRG_WITH_GRAPH
    PROP_VIEW_SHOW_GRAPH,
#endif
    PROP_MOVE_DOWN_QUEUE,
    PROP_MOVE_UP_QUEUE,
    PROP_MOVE_BOTTOM_QUEUE,
    PROP_MOVE_TOP_QUEUE,
    PROP_START_NOW
};

#define G_DATAKEY_CONF_KEY "conf-key"
#define G_DATAKEY_PREF_VALUE "pref-index"

G_DEFINE_TYPE(TrgMenuBar, trg_menu_bar, GTK_TYPE_MENU_BAR)
#define TRG_MENU_BAR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_MENU_BAR, TrgMenuBarPrivate))
typedef struct _TrgMenuBarPrivate TrgMenuBarPrivate;

struct _TrgMenuBarPrivate {
    GtkWidget *mb_connect;
    GtkWidget *mb_disconnect;
    GtkWidget *mb_add;
    GtkWidget *mb_add_url;
    GtkWidget *mb_move;
    GtkWidget *mb_remove;
    GtkWidget *mb_delete;
    GtkWidget *mb_resume;
    GtkWidget *mb_pause;
    GtkWidget *mb_resume_all;
    GtkWidget *mb_pause_all;
    GtkWidget *mb_verify;
    GtkWidget *mb_reannounce;
    GtkWidget *mb_props;
    GtkWidget *mb_local_prefs;
    GtkWidget *mb_remote_prefs;
    GtkWidget *mb_view_states;
    GtkWidget *mb_view_notebook;
    GtkWidget *mb_view_stats;
    GtkWidget *mb_about;
    GtkWidget *mb_quit;
    GtkWidget *mb_directory_filters;
    GtkWidget *mb_tracker_filters;
#if TRG_WITH_GRAPH
    GtkWidget *mb_view_graph;
#endif
    GtkWidget *mb_down_queue;
    GtkWidget *mb_up_queue;
    GtkWidget *mb_bottom_queue;
    GtkWidget *mb_top_queue;
    GtkWidget *mb_start_now;
    GtkWidget *mb_queues_seperator;
    GtkWidget *mb_view_classic;
    GtkWidget *mb_view_transmission;
    GtkWidget *mb_view_transmission_compact;
    GtkAccelGroup *accel_group;
    TrgPrefs *prefs;
    TrgMainWindow *main_window;
    TrgTorrentTreeView *torrent_tree_view;
};

void trg_menu_bar_set_supports_queues(TrgMenuBar * mb,
                                      gboolean supportsQueues)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(mb);

    gtk_widget_set_visible(priv->mb_down_queue, supportsQueues);
    gtk_widget_set_visible(priv->mb_up_queue, supportsQueues);
    gtk_widget_set_visible(priv->mb_top_queue, supportsQueues);
    gtk_widget_set_visible(priv->mb_bottom_queue, supportsQueues);
    gtk_widget_set_visible(priv->mb_queues_seperator, supportsQueues);
    gtk_widget_set_visible(priv->mb_start_now, supportsQueues);
}

void trg_menu_bar_connected_change(TrgMenuBar * mb, gboolean connected)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(mb);

    gtk_widget_set_sensitive(priv->mb_add, connected);
    gtk_widget_set_sensitive(priv->mb_add_url, connected);
    gtk_widget_set_sensitive(priv->mb_disconnect, connected);
    gtk_widget_set_sensitive(priv->mb_remote_prefs, connected);
    gtk_widget_set_sensitive(priv->mb_view_stats, connected);
    gtk_widget_set_sensitive(priv->mb_resume_all, connected);
    gtk_widget_set_sensitive(priv->mb_pause_all, connected);
}

void
trg_menu_bar_torrent_actions_sensitive(TrgMenuBar * mb, gboolean sensitive)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(mb);

    gtk_widget_set_sensitive(priv->mb_props, sensitive);
    gtk_widget_set_sensitive(priv->mb_remove, sensitive);
    gtk_widget_set_sensitive(priv->mb_delete, sensitive);
    gtk_widget_set_sensitive(priv->mb_resume, sensitive);
    gtk_widget_set_sensitive(priv->mb_pause, sensitive);
    gtk_widget_set_sensitive(priv->mb_verify, sensitive);
    gtk_widget_set_sensitive(priv->mb_reannounce, sensitive);
    gtk_widget_set_sensitive(priv->mb_move, sensitive);
    gtk_widget_set_sensitive(priv->mb_start_now, sensitive);
    gtk_widget_set_sensitive(priv->mb_up_queue, sensitive);
    gtk_widget_set_sensitive(priv->mb_down_queue, sensitive);
    gtk_widget_set_sensitive(priv->mb_top_queue, sensitive);
    gtk_widget_set_sensitive(priv->mb_bottom_queue, sensitive);
}

static void
trg_menu_bar_set_property(GObject * object,
                          guint prop_id, const GValue * value,
                          GParamSpec * pspec G_GNUC_UNUSED)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_ACCEL_GROUP:
        priv->accel_group = g_value_get_object(value);
        break;
    case PROP_PREFS:
        priv->prefs = g_value_get_object(value);
        break;
    case PROP_MAIN_WINDOW:
        priv->main_window = g_value_get_object(value);
        break;
    case PROP_TORRENT_TREE_VIEW:
        priv->torrent_tree_view = g_value_get_object(value);
        break;
    }
}

static void
trg_menu_bar_get_property(GObject * object, guint property_id,
                          GValue * value, GParamSpec * pspec)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CONNECT_BUTTON:
        g_value_set_object(value, priv->mb_connect);
        break;
    case PROP_DISCONNECT_BUTTON:
        g_value_set_object(value, priv->mb_disconnect);
        break;
    case PROP_ADD_BUTTON:
        g_value_set_object(value, priv->mb_add);
        break;
    case PROP_ADD_URL_BUTTON:
        g_value_set_object(value, priv->mb_add_url);
        break;
    case PROP_REMOVE_BUTTON:
        g_value_set_object(value, priv->mb_remove);
        break;
    case PROP_DELETE_BUTTON:
        g_value_set_object(value, priv->mb_delete);
        break;
    case PROP_MOVE_UP_QUEUE:
        g_value_set_object(value, priv->mb_up_queue);
        break;
    case PROP_MOVE_DOWN_QUEUE:
        g_value_set_object(value, priv->mb_down_queue);
        break;
    case PROP_MOVE_TOP_QUEUE:
        g_value_set_object(value, priv->mb_top_queue);
        break;
    case PROP_MOVE_BOTTOM_QUEUE:
        g_value_set_object(value, priv->mb_bottom_queue);
        break;
    case PROP_START_NOW:
        g_value_set_object(value, priv->mb_start_now);
        break;
    case PROP_MOVE_BUTTON:
        g_value_set_object(value, priv->mb_move);
        break;
    case PROP_RESUME_BUTTON:
        g_value_set_object(value, priv->mb_resume);
        break;
    case PROP_RESUME_ALL_BUTTON:
        g_value_set_object(value, priv->mb_resume_all);
        break;
    case PROP_PAUSE_BUTTON:
        g_value_set_object(value, priv->mb_pause);
        break;
    case PROP_PAUSE_ALL_BUTTON:
        g_value_set_object(value, priv->mb_pause_all);
        break;
    case PROP_VERIFY_BUTTON:
        g_value_set_object(value, priv->mb_verify);
        break;
    case PROP_REANNOUNCE_BUTTON:
        g_value_set_object(value, priv->mb_reannounce);
        break;
    case PROP_PROPS_BUTTON:
        g_value_set_object(value, priv->mb_props);
        break;
    case PROP_REMOTE_PREFS_BUTTON:
        g_value_set_object(value, priv->mb_remote_prefs);
        break;
    case PROP_LOCAL_PREFS_BUTTON:
        g_value_set_object(value, priv->mb_local_prefs);
        break;
    case PROP_ABOUT_BUTTON:
        g_value_set_object(value, priv->mb_about);
        break;
#if TRG_WITH_GRAPH
    case PROP_VIEW_SHOW_GRAPH:
        g_value_set_object(value, priv->mb_view_graph);
        break;
#endif
    case PROP_VIEW_STATES_BUTTON:
        g_value_set_object(value, priv->mb_view_states);
        break;
    case PROP_VIEW_NOTEBOOK_BUTTON:
        g_value_set_object(value, priv->mb_view_notebook);
        break;
    case PROP_VIEW_STATS_BUTTON:
        g_value_set_object(value, priv->mb_view_stats);
        break;
    case PROP_QUIT:
        g_value_set_object(value, priv->mb_quit);
        break;
    case PROP_DIR_FILTERS:
        g_value_set_object(value, priv->mb_directory_filters);
        break;
    case PROP_TRACKER_FILTERS:
        g_value_set_object(value, priv->mb_tracker_filters);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_menu_bar_install_widget_prop(GObjectClass * class, guint propId,
                                 const gchar * name, const gchar * nick)
{
    g_object_class_install_property(class,
                                    propId,
                                    g_param_spec_object(name,
                                                        nick,
                                                        nick,
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_READABLE
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));
}

GtkWidget *trg_menu_bar_item_new(GtkMenuShell * shell, const gchar * text,
                                 const gchar * stock_id,
                                 gboolean sensitive)
{
    GtkWidget *item = gtk_image_menu_item_new_with_label(stock_id);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(item), TRUE);

    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
                                              (item), TRUE);
    gtk_menu_item_set_use_underline(GTK_MENU_ITEM(item), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(item), text);
    gtk_widget_set_sensitive(item, sensitive);

    gtk_menu_shell_append(shell, item);

    return item;
}

static void
trg_menu_bar_accel_add(TrgMenuBar * menu, GtkWidget * item,
                       guint key, GdkModifierType mods)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(menu);

    gtk_widget_add_accelerator(item, "activate", priv->accel_group,
                               key, mods, GTK_ACCEL_VISIBLE);

}

static void view_menu_radio_item_toggled_cb(GtkCheckMenuItem * w,
                                            gpointer data)
{
    TrgPrefs *p = TRG_PREFS(data);
    const gchar *key =
        (gchar *) g_object_get_data(G_OBJECT(w), G_DATAKEY_CONF_KEY);

    if (gtk_check_menu_item_get_active(w)) {
        gint index =
            GPOINTER_TO_INT(g_object_get_data
                            (G_OBJECT(w), G_DATAKEY_PREF_VALUE));
        trg_prefs_set_int(p, key, index, TRG_PREFS_GLOBAL);
    }
}

static void view_menu_item_toggled_cb(GtkCheckMenuItem * w, gpointer data)
{
    TrgPrefs *p = TRG_PREFS(data);
    const gchar *key =
        (gchar *) g_object_get_data(G_OBJECT(w), G_DATAKEY_CONF_KEY);
    trg_prefs_set_bool(p, key, gtk_check_menu_item_get_active(w),
                       TRG_PREFS_GLOBAL);
}

static void
view_menu_bar_toggled_dependency_cb(GtkCheckMenuItem * w, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_check_menu_item_get_active
                             (GTK_CHECK_MENU_ITEM(w)));
}

static void
trg_menu_bar_view_item_update(TrgPrefs * p, const gchar * updatedKey,
                              gpointer data)
{
    const gchar *key =
        (gchar *) g_object_get_data(G_OBJECT(data), G_DATAKEY_CONF_KEY);
    if (!g_strcmp0(updatedKey, key))
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(data),
                                       trg_prefs_get_bool(p, key,
                                                          TRG_PREFS_GLOBAL));
}

static void
trg_menu_bar_view_radio_item_update(TrgPrefs * p, const gchar * updatedKey,
                                    gpointer data)
{
    const gchar *key =
        (gchar *) g_object_get_data(G_OBJECT(data), G_DATAKEY_CONF_KEY);
    gint myIndex =
        GPOINTER_TO_INT(g_object_get_data
                        (G_OBJECT(data), G_DATAKEY_PREF_VALUE));

    if (!g_strcmp0(updatedKey, key)) {
        gboolean shouldBeActive =
            trg_prefs_get_int(p, key, TRG_PREFS_GLOBAL) == myIndex;
        if (shouldBeActive !=
            gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(data)))
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(data),
                                           shouldBeActive);
    }
}

static GtkWidget *trg_menu_bar_view_radio_item_new(TrgPrefs * prefs,
                                                   GSList * group,
                                                   const gchar * key,
                                                   gint index,
                                                   const gchar * label)
{
    GtkWidget *w = gtk_radio_menu_item_new_with_label(group, label);
    g_object_set_data_full(G_OBJECT(w), G_DATAKEY_CONF_KEY, g_strdup(key),
                           g_free);
    g_object_set_data(G_OBJECT(w), G_DATAKEY_PREF_VALUE,
                      GINT_TO_POINTER(index));

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),
                                   trg_prefs_get_int(prefs, key,
                                                     TRG_PREFS_GLOBAL) ==
                                   (gint64) index);

    g_signal_connect(w, "toggled",
                     G_CALLBACK(view_menu_radio_item_toggled_cb), prefs);
    g_signal_connect(prefs, "pref-changed",
                     G_CALLBACK(trg_menu_bar_view_radio_item_update), w);

    return w;
}

static GtkWidget *trg_menu_bar_view_item_new(TrgPrefs * prefs,
                                             const gchar * key,
                                             const gchar * label,
                                             GtkWidget * dependency)
{
    GtkWidget *w = gtk_check_menu_item_new_with_label(label);
    g_object_set_data_full(G_OBJECT(w), G_DATAKEY_CONF_KEY, g_strdup(key),
                           g_free);

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),
                                   trg_prefs_get_bool(prefs, key,
                                                      TRG_PREFS_GLOBAL));

    if (dependency) {
        gtk_widget_set_sensitive(w,
                                 gtk_check_menu_item_get_active
                                 (GTK_CHECK_MENU_ITEM(dependency)));
        g_signal_connect(dependency, "toggled",
                         G_CALLBACK(view_menu_bar_toggled_dependency_cb),
                         w);
    }

    g_signal_connect(w, "toggled",
                     G_CALLBACK(view_menu_item_toggled_cb), prefs);
    g_signal_connect(prefs, "pref-changed",
                     G_CALLBACK(trg_menu_bar_view_item_update), w);

    return w;
}

static GtkWidget *trg_menu_bar_view_menu_new(TrgMenuBar * mb)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(mb);

    GtkWidget *view = gtk_menu_item_new_with_mnemonic(_("_View"));
    GtkWidget *viewMenu = gtk_menu_new();
    GSList *group;

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), viewMenu);

    priv->mb_view_transmission =
        trg_menu_bar_view_radio_item_new(priv->prefs, NULL,
                                         TRG_PREFS_KEY_STYLE, TRG_STYLE_TR,
                                         _("Transmission Style"));
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
                          priv->mb_view_transmission);
    group =
        gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM
                                      (priv->mb_view_transmission));
    priv->mb_view_transmission_compact =
        trg_menu_bar_view_radio_item_new(priv->prefs, group,
                                         TRG_PREFS_KEY_STYLE,
                                         TRG_STYLE_TR_COMPACT,
                                         _("Transmission Compact Style"));
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
                          priv->mb_view_transmission_compact);
    group =
        gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM
                                      (priv->mb_view_transmission_compact));
    priv->mb_view_classic =
        trg_menu_bar_view_radio_item_new(priv->prefs, group,
                                         TRG_PREFS_KEY_STYLE,
                                         TRG_STYLE_CLASSIC,
                                         _("Classic Style"));
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), priv->mb_view_classic);

    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
                          trg_tree_view_sort_menu(TRG_TREE_VIEW
                                                  (priv->torrent_tree_view),
                                                  _("Sort")));

    priv->mb_view_states =
        trg_menu_bar_view_item_new(priv->prefs,
                                   TRG_PREFS_KEY_SHOW_STATE_SELECTOR,
                                   _("State selector"), NULL);
    trg_menu_bar_accel_add(mb, priv->mb_view_states, GDK_F2, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), priv->mb_view_states);

    priv->mb_directory_filters =
        trg_menu_bar_view_item_new(priv->prefs, TRG_PREFS_KEY_FILTER_DIRS,
                                   _("Directory filters"),
                                   priv->mb_view_states);
    trg_menu_bar_accel_add(mb, priv->mb_directory_filters, GDK_F3, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
                          priv->mb_directory_filters);

    priv->mb_tracker_filters =
        trg_menu_bar_view_item_new(priv->prefs,
                                   TRG_PREFS_KEY_FILTER_TRACKERS,
                                   _("Tracker filters"),
                                   priv->mb_view_states);
    trg_menu_bar_accel_add(mb, priv->mb_tracker_filters, GDK_F4, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
                          priv->mb_tracker_filters);

    priv->mb_view_notebook =
        trg_menu_bar_view_item_new(priv->prefs,
                                   TRG_PREFS_KEY_SHOW_NOTEBOOK,
                                   _("Torrent Details"), NULL);
    trg_menu_bar_accel_add(mb, priv->mb_view_notebook, GDK_F5, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
                          priv->mb_view_notebook);

#if TRG_WITH_GRAPH
    priv->mb_view_graph =
        trg_menu_bar_view_item_new(priv->prefs, TRG_PREFS_KEY_SHOW_GRAPH,
                                   _("Graph"), priv->mb_view_notebook);
    trg_menu_bar_accel_add(mb, priv->mb_view_graph, GDK_F6, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), priv->mb_view_graph);
#endif

    priv->mb_view_stats =
        gtk_menu_item_new_with_mnemonic(_("_Statistics"));
    trg_menu_bar_accel_add(mb, priv->mb_view_stats, GDK_F7, 0);
    gtk_widget_set_sensitive(priv->mb_view_stats, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), priv->mb_view_stats);

    return view;
}

static GtkWidget *trg_menu_bar_options_menu_new(TrgMenuBar * menu)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(menu);

    GtkWidget *opts = gtk_menu_item_new_with_mnemonic(_("_Options"));
    GtkWidget *optsMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(opts), optsMenu);

    priv->mb_local_prefs =
        trg_menu_bar_item_new(GTK_MENU_SHELL(optsMenu),
                              _("_Local Preferences"),
                              GTK_STOCK_PREFERENCES, TRUE);
    trg_menu_bar_accel_add(menu, priv->mb_local_prefs, GDK_s,
                           GDK_CONTROL_MASK);

    priv->mb_remote_prefs =
        trg_menu_bar_item_new(GTK_MENU_SHELL(optsMenu),
                              _("_Remote Preferences"),
                              GTK_STOCK_NETWORK, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_remote_prefs, GDK_s,
                           GDK_MOD1_MASK);

    return opts;
}

static void
trg_menu_bar_file_connect_item_new(TrgMainWindow * win,
                                   GtkMenuShell * shell,
                                   const gchar * text,
                                   gboolean checked, JsonObject * profile)
{
    GtkWidget *item = gtk_check_menu_item_new_with_label(text);

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), checked);
    g_object_set_data(G_OBJECT(item), "profile", profile);
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);

    g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(connect_cb),
                     win);

    gtk_menu_shell_append(shell, item);
}

GtkWidget *trg_menu_bar_file_connect_menu_new(TrgMainWindow * win,
                                              TrgPrefs * p)
{
    GtkWidget *menu = gtk_menu_new();
    GList *profiles = json_array_get_elements(trg_prefs_get_profiles(p));
    JsonObject *currentProfile = trg_prefs_get_profile(p);
    GList *li;

    for (li = profiles; li; li = g_list_next(li)) {
        JsonObject *profile = json_node_get_object((JsonNode *) li->data);
        const gchar *name_value;

        if (json_object_has_member(profile, TRG_PREFS_KEY_PROFILE_NAME)) {
            name_value = json_object_get_string_member(profile,
                                                       TRG_PREFS_KEY_PROFILE_NAME);
        } else {
            name_value = _(TRG_PROFILE_NAME_DEFAULT);
        }

        trg_menu_bar_file_connect_item_new(win, GTK_MENU_SHELL(menu),
                                           name_value,
                                           profile == currentProfile,
                                           profile);
    }

    g_list_free(profiles);

    return menu;
}

static GtkWidget *trg_menu_bar_file_file_menu_new(TrgMenuBar * menu)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(menu);

    GtkWidget *file = gtk_menu_item_new_with_mnemonic(_("_File"));
    GtkWidget *fileMenu = gtk_menu_new();

    GtkWidget *connectMenu =
        trg_menu_bar_file_connect_menu_new(priv->main_window, priv->prefs);

    priv->mb_connect =
        trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), _("Connect"),
                              GTK_STOCK_CONNECT, TRUE);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(priv->mb_connect),
                              connectMenu);

    priv->mb_disconnect =
        trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), _("_Disconnect"),
                              GTK_STOCK_DISCONNECT, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_disconnect, GDK_d,
                           GDK_CONTROL_MASK);

    priv->mb_add =
        trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), _("_Add"),
                              GTK_STOCK_ADD, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_add, GDK_o, GDK_CONTROL_MASK);

    priv->mb_add_url =
        trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), _("Add from _URL"),
                              GTK_STOCK_ADD, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_add_url, GDK_u,
                           GDK_CONTROL_MASK);

    priv->mb_quit =
        trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), _("_Quit"),
                              GTK_STOCK_QUIT, TRUE);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), fileMenu);

    return file;
}

static GtkWidget *trg_menu_bar_torrent_menu_new(TrgMenuBar * menu)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(menu);
    GtkWidget *torrent = gtk_menu_item_new_with_mnemonic(_("_Torrent"));
    GtkWidget *torrentMenu = gtk_menu_new();

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(torrent), torrentMenu);

    priv->mb_props =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                              _("Properties"), GTK_STOCK_PROPERTIES,
                              FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_props, GDK_i, GDK_CONTROL_MASK);

    priv->mb_resume =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), _("_Resume"),
                              GTK_STOCK_MEDIA_PLAY, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_resume, GDK_r, GDK_CONTROL_MASK);

    priv->mb_pause =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), _("_Pause"),
                              GTK_STOCK_MEDIA_PAUSE, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_pause, GDK_p, GDK_CONTROL_MASK);

    priv->mb_verify =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), _("_Verify"),
                              GTK_STOCK_REFRESH, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_verify, GDK_h, GDK_CONTROL_MASK);

    priv->mb_reannounce =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                              _("Re-_announce"), GTK_STOCK_REFRESH, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_reannounce, GDK_q,
                           GDK_CONTROL_MASK);

    priv->mb_move =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), _("_Move"),
                              GTK_STOCK_HARDDISK, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_move, GDK_m, GDK_CONTROL_MASK);

    priv->mb_remove =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), _("Remove"),
                              GTK_STOCK_REMOVE, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_remove, GDK_Delete, 0);

    priv->mb_delete =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                              _("Remove and Delete"), GTK_STOCK_CLEAR,
                              FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_delete, GDK_Delete,
                           GDK_SHIFT_MASK);

    priv->mb_queues_seperator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(torrentMenu),
                          priv->mb_queues_seperator);

    priv->mb_start_now = trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                                               _("Start Now"),
                                               GTK_STOCK_MEDIA_PLAY,
                                               FALSE);

    priv->mb_up_queue = trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                                              _("Move Up Queue"),
                                              GTK_STOCK_GO_UP, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_up_queue, GDK_Up,
                           GDK_SHIFT_MASK);

    priv->mb_down_queue =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                              _("Move Down Queue"), GTK_STOCK_GO_DOWN,
                              FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_down_queue, GDK_Down,
                           GDK_SHIFT_MASK);

    priv->mb_bottom_queue =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                              _("Bottom Of Queue"), GTK_STOCK_GOTO_BOTTOM,
                              FALSE);

    priv->mb_top_queue = trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                                               _("Top Of Queue"),
                                               GTK_STOCK_GOTO_TOP, FALSE);

    gtk_menu_shell_append(GTK_MENU_SHELL(torrentMenu),
                          gtk_separator_menu_item_new());

    priv->mb_resume_all =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
                              _("_Resume All"), GTK_STOCK_MEDIA_PLAY,
                              FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_resume_all, GDK_r,
                           GDK_SHIFT_MASK | GDK_CONTROL_MASK);

    priv->mb_pause_all =
        trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), _("_Pause All"),
                              GTK_STOCK_MEDIA_PAUSE, FALSE);
    trg_menu_bar_accel_add(menu, priv->mb_pause_all, GDK_p,
                           GDK_SHIFT_MASK | GDK_CONTROL_MASK);

    return torrent;
}

static GtkWidget *trg_menu_bar_help_menu_new(TrgMenuBar * menuBar)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(menuBar);

    GtkWidget *help = gtk_menu_item_new_with_mnemonic(_("_Help"));
    GtkWidget *helpMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), help);

    priv->mb_about =
        trg_menu_bar_item_new(GTK_MENU_SHELL(helpMenu), _("_About"),
                              GTK_STOCK_ABOUT, TRUE);

    return helpMenu;
}

static void menu_bar_refresh_menu(GtkWidget * w, gpointer data)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(data);
    GtkWidget *old =
        gtk_menu_item_get_submenu(GTK_MENU_ITEM(priv->mb_connect));
    GtkWidget *new =
        trg_menu_bar_file_connect_menu_new(priv->main_window, priv->prefs);

    gtk_widget_destroy(old);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(priv->mb_connect), new);
    gtk_widget_show_all(new);
}

static GObject *trg_menu_bar_constructor(GType type,
                                         guint n_construct_properties,
                                         GObjectConstructParam *
                                         construct_params)
{
    GObject *object;
    TrgMenuBarPrivate *priv;
    TrgMenuBar *menu;

    object = G_OBJECT_CLASS
        (trg_menu_bar_parent_class)->constructor(type,
                                                 n_construct_properties,
                                                 construct_params);
    menu = TRG_MENU_BAR(object);
    priv = TRG_MENU_BAR_GET_PRIVATE(object);

    gtk_menu_shell_append(GTK_MENU_SHELL(object),
                          trg_menu_bar_file_file_menu_new(menu));
    gtk_menu_shell_append(GTK_MENU_SHELL(object),
                          trg_menu_bar_torrent_menu_new(menu));
    gtk_menu_shell_append(GTK_MENU_SHELL(object),
                          trg_menu_bar_options_menu_new(menu));
    gtk_menu_shell_append(GTK_MENU_SHELL(object),
                          trg_menu_bar_view_menu_new(menu));
    trg_menu_bar_help_menu_new(TRG_MENU_BAR(object));

    g_signal_connect(G_OBJECT(priv->prefs), "pref-profile-changed",
                     G_CALLBACK(menu_bar_refresh_menu), object);

    return object;
}

static void trg_menu_bar_class_init(TrgMenuBarClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = trg_menu_bar_get_property;
    object_class->set_property = trg_menu_bar_set_property;
    object_class->constructor = trg_menu_bar_constructor;

    g_type_class_add_private(klass, sizeof(TrgMenuBarPrivate));

    trg_menu_bar_install_widget_prop(object_class, PROP_CONNECT_BUTTON,
                                     "connect-button", "Connect Button");
    trg_menu_bar_install_widget_prop(object_class,
                                     PROP_DISCONNECT_BUTTON,
                                     "disconnect-button",
                                     "Disconnect Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_ADD_BUTTON,
                                     "add-button", "Add Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_ADD_URL_BUTTON,
                                     "add-url-button", "Add URL Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_REMOVE_BUTTON,
                                     "remove-button", "Remove Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_MOVE_BUTTON,
                                     "move-button", "Move Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_DELETE_BUTTON,
                                     "delete-button", "Delete Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_RESUME_BUTTON,
                                     "resume-button", "Resume Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_RESUME_ALL_BUTTON,
                                     "resume-all-button",
                                     "Resume All Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_VERIFY_BUTTON,
                                     "verify-button", "Verify Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_REANNOUNCE_BUTTON,
                                     "reannounce-button",
                                     "Re-announce Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_PAUSE_ALL_BUTTON,
                                     "pause-all-button",
                                     "Pause All Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_PAUSE_BUTTON,
                                     "pause-button", "Pause Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_PROPS_BUTTON,
                                     "props-button", "Props Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_ABOUT_BUTTON,
                                     "about-button", "About Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_VIEW_STATS_BUTTON,
                                     "view-stats-button",
                                     "View stats button");
    trg_menu_bar_install_widget_prop(object_class, PROP_VIEW_STATES_BUTTON,
                                     "view-states-button",
                                     "View states Button");
    trg_menu_bar_install_widget_prop(object_class,
                                     PROP_VIEW_NOTEBOOK_BUTTON,
                                     "view-notebook-button",
                                     "View notebook Button");
    trg_menu_bar_install_widget_prop(object_class,
                                     PROP_REMOTE_PREFS_BUTTON,
                                     "remote-prefs-button",
                                     "Remote Prefs Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_LOCAL_PREFS_BUTTON,
                                     "local-prefs-button",
                                     "Local Prefs Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_QUIT,
                                     "quit-button", "Quit Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_DIR_FILTERS,
                                     "dir-filters", "Dir Filters");
    trg_menu_bar_install_widget_prop(object_class, PROP_TRACKER_FILTERS,
                                     "tracker-filters", "Tracker Filters");
#if TRG_WITH_GRAPH
    trg_menu_bar_install_widget_prop(object_class, PROP_VIEW_SHOW_GRAPH,
                                     "show-graph", "Show Graph");
#endif
    trg_menu_bar_install_widget_prop(object_class, PROP_MOVE_DOWN_QUEUE,
                                     "down-queue", "Down Queue");
    trg_menu_bar_install_widget_prop(object_class, PROP_MOVE_UP_QUEUE,
                                     "up-queue", "Up Queue");
    trg_menu_bar_install_widget_prop(object_class, PROP_MOVE_BOTTOM_QUEUE,
                                     "bottom-queue", "Bottom Queue");
    trg_menu_bar_install_widget_prop(object_class, PROP_MOVE_TOP_QUEUE,
                                     "top-queue", "Top Queue");
    trg_menu_bar_install_widget_prop(object_class, PROP_START_NOW,
                                     "start-now", "Start Now");

    g_object_class_install_property(object_class,
                                    PROP_PREFS,
                                    g_param_spec_object("prefs",
                                                        "prefs",
                                                        "Prefs",
                                                        TRG_TYPE_PREFS,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_ACCEL_GROUP,
                                    g_param_spec_object("accel-group",
                                                        "accel-group",
                                                        "accel-group",
                                                        GTK_TYPE_ACCEL_GROUP,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_MAIN_WINDOW,
                                    g_param_spec_object("mainwin",
                                                        "mainwin",
                                                        "mainwin",
                                                        TRG_TYPE_MAIN_WINDOW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_TORRENT_TREE_VIEW,
                                    g_param_spec_object
                                    ("torrent-tree-view",
                                     "torrent-tree-view",
                                     "torrent-tree-view",
                                     TRG_TYPE_TORRENT_TREE_VIEW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));
}

static void trg_menu_bar_init(TrgMenuBar * self)
{
}

TrgMenuBar *trg_menu_bar_new(TrgMainWindow * win, TrgPrefs * prefs,
                             TrgTorrentTreeView * ttv,
                             GtkAccelGroup * accel_group)
{
    return g_object_new(TRG_TYPE_MENU_BAR,
                        "torrent-tree-view", ttv, "prefs", prefs,
                        "mainwin", win, "accel-group", accel_group, NULL);
}
