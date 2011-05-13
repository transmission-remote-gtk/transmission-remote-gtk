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

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include "hig.h"
#include "trg-preferences-dialog.h"
#include "trg-json-widgets.h"
#include "trg-main-window.h"
#include "trg-preferences.h"
#include "util.h"

#define TRG_PREFERENCES_DIALOG_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), TRG_TYPE_PREFERENCES_DIALOG, TrgPreferencesDialogPrivate))

G_DEFINE_TYPE(TrgPreferencesDialog, trg_preferences_dialog,
              GTK_TYPE_DIALOG)

enum {
    PROP_0,
    PROP_TRG_CLIENT,
    PROP_MAIN_WINDOW
};

#define GCONF_OBJECT_KEY        "gconf-key"

struct _TrgPreferencesDialogPrivate {
    TrgMainWindow *win;
    trg_client *client;
};

static GObject *instance = NULL;

static void
trg_preferences_dialog_set_property(GObject * object,
                                    guint prop_id,
                                    const GValue * value,
                                    GParamSpec * pspec G_GNUC_UNUSED)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_MAIN_WINDOW:
        priv->win = g_value_get_object(value);
        break;
    case PROP_TRG_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    }
}

static void
trg_preferences_response_cb(GtkDialog * dlg, gint res_id,
                            gpointer data G_GNUC_UNUSED)
{
    gtk_widget_destroy(GTK_WIDGET(dlg));
    instance = NULL;
}

static void
trg_preferences_dialog_get_property(GObject * object,
                                    guint prop_id,
                                    GValue * value,
                                    GParamSpec * pspec G_GNUC_UNUSED)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_MAIN_WINDOW:
        g_value_set_object(value, priv->win);
        break;
    case PROP_TRG_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    }
}

static void update_activeonly_cb(GtkToggleButton * w, gpointer data)
{
    trg_client *client = (trg_client *) data;
    client->activeOnlyUpdate = gtk_toggle_button_get_active(w);
}

static void toggled_cb(GtkToggleButton * w, gpointer gconf)
{
    const char *key = g_object_get_data(G_OBJECT(w), GCONF_OBJECT_KEY);
    gboolean flag = gtk_toggle_button_get_active(w);

    gconf_client_set_bool(GCONF_CLIENT(gconf), key, flag, NULL);
}

static GtkWidget *new_check_button(GConfClient * gconf,
                                   const char *mnemonic, const char *key)
{
    GtkWidget *w = gtk_check_button_new_with_mnemonic(mnemonic);

    g_object_set_data_full(G_OBJECT(w), GCONF_OBJECT_KEY,
                           g_strdup(key), g_free);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 gconf_client_get_bool(gconf, key, NULL));
    g_signal_connect(w, "toggled", G_CALLBACK(toggled_cb), gconf);
    return w;
}

static void interval_changed_cb(GtkWidget * w, gpointer data)
{
    trg_client *client = (trg_client *) data;
    client->interval =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
}

static void spun_cb_int(GtkWidget * widget, gpointer gconf)
{
    gchar *key = g_object_get_data(G_OBJECT(widget), GCONF_OBJECT_KEY);

    gconf_client_set_int(GCONF_CLIENT(gconf),
                         key,
                         gtk_spin_button_get_value_as_int
                         (GTK_SPIN_BUTTON(widget)), NULL);
}

static GtkWidget *new_spin_button(GConfClient * gconf,
                                  const gchar * key,
                                  int low, int high, int step, int deflt)
{
    GtkWidget *w;
    gint currentValue;

    w = gtk_spin_button_new_with_range(low, high, step);
    g_object_set_data_full(G_OBJECT(w), GCONF_OBJECT_KEY,
                           g_strdup(key), g_free);

    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(w), 0);

    currentValue =
        gconf_client_get_int_or_default(gconf, key, deflt, NULL);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), currentValue);

    g_signal_connect(w, "value-changed", G_CALLBACK(spun_cb_int), gconf);

    return w;
}

static void entry_changed_cb(GtkEntry * w, gpointer gconf)
{
    const char *key, *value;

    key = g_object_get_data(G_OBJECT(w), GCONF_OBJECT_KEY);
    value = gtk_entry_get_text(w);

    gconf_client_set_string(GCONF_CLIENT(gconf), key, value, NULL);
}

static GtkWidget *new_entry(GConfClient * gconf, const char *key)
{
    GtkWidget *w;
    const char *value;

    w = gtk_entry_new();
    value = gconf_client_get_string(gconf, key, NULL);

    if (value != NULL) {
        gtk_entry_set_text(GTK_ENTRY(w), value);
        g_free((gpointer) value);
    }

    g_object_set_data_full(G_OBJECT(w), GCONF_OBJECT_KEY,
                           g_strdup(key), g_free);

    g_signal_connect(w, "changed", G_CALLBACK(entry_changed_cb), gconf);
    return w;
}

static void toggle_filter_trackers(GtkToggleButton * w, gpointer win)
{
    TrgStateSelector *selector =
        trg_main_window_get_state_selector(TRG_MAIN_WINDOW(win));
    trg_state_selector_set_show_trackers(selector,
                                         gtk_toggle_button_get_active(w));
}

static void toggle_filter_dirs(GtkToggleButton * w, gpointer win)
{
    TrgStateSelector *selector =
        trg_main_window_get_state_selector(TRG_MAIN_WINDOW(win));
    trg_state_selector_set_show_dirs(selector,
                                     gtk_toggle_button_get_active(w));
}

static void toggle_graph(GtkToggleButton * w, gpointer win)
{
    if (gtk_toggle_button_get_active(w))
        trg_main_window_add_graph(TRG_MAIN_WINDOW(win), TRUE);
    else
        trg_main_window_remove_graph(TRG_MAIN_WINDOW(win));
}

static void toggle_tray_icon(GtkToggleButton * w, gpointer win)
{
    if (gtk_toggle_button_get_active(w))
        trg_main_window_add_status_icon(TRG_MAIN_WINDOW(win));
    else
        trg_main_window_remove_status_icon(TRG_MAIN_WINDOW(win));
}

static GtkWidget *trg_prefs_desktopPage(trg_client * client,
                                        TrgMainWindow * win)
{
    GtkWidget *tray, *w, *t;
    gint row = 0;

    GConfClient *gconf = client->gconf;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Features"));

    w = new_check_button(gconf, _("Directory filters"),
                         TRG_GCONF_KEY_FILTER_DIRS);
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(toggle_filter_dirs), win);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_check_button(gconf, _("Tracker filters"),
                         TRG_GCONF_KEY_FILTER_TRACKERS);
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(toggle_filter_trackers), win);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_check_button(gconf, _("Show graph"), TRG_GCONF_KEY_SHOW_GRAPH);
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(toggle_graph), win);
    hig_workarea_add_wide_control(t, &row, w);

    hig_workarea_add_section_title(t, &row, _("System Tray"));

    tray = new_check_button(gconf, _("Show in system tray"),
                            TRG_GCONF_KEY_SYSTEM_TRAY);
    g_signal_connect(G_OBJECT(tray), "toggled",
                     G_CALLBACK(toggle_tray_icon), win);
    hig_workarea_add_wide_control(t, &row, tray);

    w = new_check_button(gconf, _("Minimise to system tray"),
                         TRG_GCONF_KEY_SYSTEM_TRAY_MINIMISE);
    gtk_widget_set_sensitive(w,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
                                                          (tray)));
    g_signal_connect(G_OBJECT(tray), "toggled",
                     G_CALLBACK(toggle_active_arg_is_sensitive), w);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_check_button(gconf, _("Torrent added notifications"),
                         TRG_GCONF_KEY_ADD_NOTIFY);
    gtk_widget_set_sensitive(w,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
                                                          (tray)));
    g_signal_connect(G_OBJECT(tray), "toggled",
                     G_CALLBACK(toggle_active_arg_is_sensitive), w);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_check_button(gconf, _("Torrent complete notifications"),
                         TRG_GCONF_KEY_COMPLETE_NOTIFY);
    gtk_widget_set_sensitive(w,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
                                                          (tray)));
    g_signal_connect(G_OBJECT(tray), "toggled",
                     G_CALLBACK(toggle_active_arg_is_sensitive), w);
    hig_workarea_add_wide_control(t, &row, w);

    return t;
}

static GtkWidget *trg_prefs_behaviorPage(trg_client * client)
{
    GtkWidget *w, *t;
    gint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Torrents"));

    w = new_check_button(client->gconf, _("Start paused"),
                         TRG_GCONF_KEY_START_PAUSED);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_check_button(client->gconf, _("Options dialog on add"),
                         TRG_GCONF_KEY_ADD_OPTIONS_DIALOG);
    hig_workarea_add_wide_control(t, &row, w);

    return t;
}

static GtkWidget *trg_prefs_serverPage(trg_client * client)
{
    GtkWidget *w, *t;
    gint row = 0;

    GConfClient *gconf = client->gconf;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Server"));

    w = new_entry(gconf, TRG_GCONF_KEY_HOSTNAME);
    hig_workarea_add_row(t, &row, _("Host:"), w, NULL);

    w = new_spin_button(gconf, TRG_GCONF_KEY_PORT, 1, 65535, 1,
                        TRG_PORT_DEFAULT);
    hig_workarea_add_row(t, &row, _("Port:"), w, NULL);

    w = new_check_button(gconf, _("Automatically connect"),
                         TRG_GCONF_KEY_AUTO_CONNECT);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_check_button(gconf, _("SSL"), TRG_GCONF_KEY_SSL);
    hig_workarea_add_wide_control(t, &row, w);

    w = new_spin_button(gconf, TRG_GCONF_KEY_UPDATE_INTERVAL, 1, 60, 1,
                        TRG_INTERVAL_DEFAULT);
    g_signal_connect(w, "value-changed", G_CALLBACK(interval_changed_cb),
                     client);
    hig_workarea_add_row(t, &row, _("Update interval:"), w, NULL);

    w = new_check_button(gconf, _("Update active torrents only"),
                         TRG_GCONF_KEY_UPDATE_ACTIVE_ONLY);
    g_signal_connect(w, "toggled", G_CALLBACK(update_activeonly_cb),
                     client);
    hig_workarea_add_wide_control(t, &row, w);

    hig_workarea_add_section_divider(t, &row);
    hig_workarea_add_section_title(t, &row, _("Authentication"));

    w = new_entry(gconf, TRG_GCONF_KEY_USERNAME);
    hig_workarea_add_row(t, &row, _("Username:"), w, NULL);

    w = new_entry(gconf, TRG_GCONF_KEY_PASSWORD);
    gtk_entry_set_visibility(GTK_ENTRY(w), FALSE);
    hig_workarea_add_row(t, &row, _("Password:"), w, NULL);

    return t;
}

static GObject *trg_preferences_dialog_constructor(GType type,
                                                   guint
                                                   n_construct_properties,
                                                   GObjectConstructParam *
                                                   construct_params)
{
    GObject *object;
    TrgPreferencesDialogPrivate *priv;
    GtkWidget *notebook;

    object =
        G_OBJECT_CLASS
        (trg_preferences_dialog_parent_class)->constructor(type,
                                                           n_construct_properties,
                                                           construct_params);
    priv = TRG_PREFERENCES_DIALOG_GET_PRIVATE(object);

    gtk_window_set_transient_for(GTK_WINDOW(object),
                                 GTK_WINDOW(priv->win));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(object), TRUE);
    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_CLOSE,
                          GTK_RESPONSE_CLOSE);

    gtk_dialog_set_default_response(GTK_DIALOG(object),
                                    GTK_RESPONSE_CLOSE);

    gtk_window_set_title(GTK_WINDOW(object), _("Local Preferences"));
    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    g_signal_connect(G_OBJECT(object),
                     "response",
                     G_CALLBACK(trg_preferences_response_cb), NULL);

    notebook = gtk_notebook_new();

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_serverPage(priv->client),
                             gtk_label_new(_("Connection")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_desktopPage(priv->client,
                                                   priv->win),
                             gtk_label_new(_("Desktop")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_behaviorPage(priv->client),
                             gtk_label_new(_("Behavior")));

    gtk_container_set_border_width(GTK_CONTAINER(notebook), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(object)->vbox), notebook,
                       TRUE, TRUE, 0);

    gtk_widget_show_all(GTK_DIALOG(object)->vbox);

    return object;
}

static void trg_preferences_dialog_init(TrgPreferencesDialog * pref_dlg)
{
}

static void
trg_preferences_dialog_class_init(TrgPreferencesDialogClass * class)
{
    GObjectClass *g_object_class = (GObjectClass *) class;

    g_object_class->constructor = trg_preferences_dialog_constructor;
    g_object_class->set_property = trg_preferences_dialog_set_property;
    g_object_class->get_property = trg_preferences_dialog_get_property;

    g_object_class_install_property(g_object_class,
                                    PROP_TRG_CLIENT,
                                    g_param_spec_pointer
                                    ("trg-client", "TClient",
                                     "Client",
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(g_object_class,
                                    PROP_MAIN_WINDOW,
                                    g_param_spec_object
                                    ("main-window", "Main Window",
                                     "Main Window", TRG_TYPE_MAIN_WINDOW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_type_class_add_private(g_object_class,
                             sizeof(TrgPreferencesDialogPrivate));
}

GtkWidget *trg_preferences_dialog_get_instance(TrgMainWindow * win,
                                               trg_client * client)
{
    if (instance == NULL) {
        instance = g_object_new(TRG_TYPE_PREFERENCES_DIALOG,
                                "main-window", win,
                                "trg-client", client, NULL);
    }

    return GTK_WIDGET(instance);
}
