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
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include "hig.h"
#include "trg-json-widgets.h"
#include "trg-preferences-dialog.h"
#include "trg-persistent-tree-view.h"
#include "trg-main-window.h"
#include "trg-prefs.h"
#include "util.h"

/* UI frontend to modify configurables stored in TrgConf.
 * To avoid lots of repetitive code, use our own functions for creating widgets
 * which also create a trg_pref_widget_desc structure and add it to a list.
 * This contains details of the config key, config flags etc, and a function
 * pointer for a save/display function. These are all called on save/load.
 */

#define TRG_PREFERENCES_DIALOG_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), TRG_TYPE_PREFERENCES_DIALOG, TrgPreferencesDialogPrivate))

G_DEFINE_TYPE(TrgPreferencesDialog, trg_preferences_dialog,
              GTK_TYPE_DIALOG)
enum {
    PROP_0, PROP_TRG_CLIENT, PROP_MAIN_WINDOW
};

struct _TrgPreferencesDialogPrivate {
    TrgMainWindow *win;
    TrgClient *client;
    TrgPrefs *prefs;
    GtkWidget *profileDelButton;
    GtkWidget *profileComboBox;
    GtkWidget *profileNameEntry;
    GtkWidget *fullUpdateCheck;
    GList *widgets;
};

static GObject *instance = NULL;

static void trg_pref_widget_desc_free(trg_pref_widget_desc * wd)
{
    g_free(wd->key);
    g_free(wd);
}

trg_pref_widget_desc *trg_pref_widget_desc_new(GtkWidget * w, gchar * key,
                                               int flags)
{
    trg_pref_widget_desc *desc = g_new0(trg_pref_widget_desc, 1);
    desc->widget = w;
    desc->key = g_strdup(key);
    desc->flags = flags;
    return desc;
}

static void
trg_pref_widget_refresh(TrgPreferencesDialog * dlg,
                        trg_pref_widget_desc * wd)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);

    wd->refreshFunc(priv->prefs, wd);
}

static void trg_pref_widget_refresh_all(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);

    GList *li;
    for (li = priv->widgets; li; li = g_list_next(li))
        trg_pref_widget_refresh(dlg, (trg_pref_widget_desc *) li->data);
}

static void
trg_pref_widget_save(TrgPreferencesDialog * dlg, trg_pref_widget_desc * wd)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);

    wd->saveFunc(priv->prefs, wd);
}

static void trg_pref_widget_save_all(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    GList *li;

    if (trg_prefs_get_profile(priv->prefs) == NULL)
        return;

    trg_client_configlock(priv->client);
    for (li = priv->widgets; li; li = g_list_next(li)) {
        trg_pref_widget_desc *wd = (trg_pref_widget_desc *) li->data;
        trg_pref_widget_save(dlg, wd);
    }
    trg_client_configunlock(priv->client);
}

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
        priv->prefs = trg_client_get_prefs(priv->client);
        break;
    }
}

static void
trg_preferences_response_cb(GtkDialog * dlg, gint res_id,
                            gpointer data G_GNUC_UNUSED)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    GList *li;

    if (res_id == GTK_RESPONSE_OK) {
        trg_pref_widget_save_all(TRG_PREFERENCES_DIALOG(dlg));
        trg_prefs_save(priv->prefs);
    }

    trg_main_window_reload_dir_aliases(priv->win);

    for (li = priv->widgets; li; li = g_list_next(li))
        trg_pref_widget_desc_free((trg_pref_widget_desc *) li->data);
    g_list_free(priv->widgets);

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

static void entry_refresh(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    gchar *value = trg_prefs_get_string(prefs, wd->key, wd->flags);

    if (value) {
        gtk_entry_set_text(GTK_ENTRY(wd->widget), value);
        g_free(value);
    } else {
        gtk_entry_set_text(GTK_ENTRY(wd->widget), "");
    }
}

static void entry_save(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;

    trg_prefs_set_string(prefs, wd->key,
                         gtk_entry_get_text(GTK_ENTRY(wd->widget)),
                         wd->flags);
}

static GtkWidget *trgp_entry_new(TrgPreferencesDialog * dlg, gchar * key,
                                 int flags)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    GtkWidget *w = gtk_entry_new();
    trg_pref_widget_desc *wd = trg_pref_widget_desc_new(w, key, flags);

    wd->saveFunc = &entry_save;
    wd->refreshFunc = &entry_refresh;

    entry_refresh(priv->prefs, wd);
    priv->widgets = g_list_append(priv->widgets, wd);

    return w;
}

static void check_refresh(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    gboolean value = trg_prefs_get_bool(prefs, wd->key, wd->flags);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wd->widget), value);
}

static void check_save(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    trg_prefs_set_bool(prefs, wd->key,
                       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
                                                    (wd->widget)),
                       wd->flags);
}

static void trgp_toggle_dependent(GtkToggleButton * b, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_toggle_button_get_active(b));
}

static GtkWidget *trgp_check_new(TrgPreferencesDialog * dlg,
                                 const char *mnemonic, gchar * key,
                                 int flags, GtkToggleButton * dependency)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);

    GtkWidget *w = gtk_check_button_new_with_mnemonic(mnemonic);

    trg_pref_widget_desc *wd = trg_pref_widget_desc_new(w, key, flags);
    wd->saveFunc = &check_save;
    wd->refreshFunc = &check_refresh;
    check_refresh(priv->prefs, wd);

    if (dependency) {
        g_signal_connect(dependency, "toggled",
                         G_CALLBACK(trgp_toggle_dependent), w);
        gtk_widget_set_sensitive(w,
                                 gtk_toggle_button_get_active(dependency));
    }

    priv->widgets = g_list_append(priv->widgets, wd);

    return w;
}

static void spin_refresh(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    GtkWidget *widget = wd->widget;

    gint value = trg_prefs_get_int(prefs, wd->key, wd->flags);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), value);
}

static void spin_save(TrgPrefs * prefs, void *wdp)
{
    trg_pref_widget_desc *wd = (trg_pref_widget_desc *) wdp;
    trg_prefs_set_int(prefs, wd->key,
                      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON
                                                       (wd->widget)),
                      wd->flags);
}

static GtkWidget *trgp_spin_new(TrgPreferencesDialog * dlg, gchar * key,
                                int low, int high, int step, int flags,
                                GtkToggleButton * dependency)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    GtkWidget *w = gtk_spin_button_new_with_range(low, high, step);
    trg_pref_widget_desc *wd = trg_pref_widget_desc_new(w, key, flags);

    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(w), 0);

    wd->saveFunc = &spin_save;
    wd->refreshFunc = &spin_refresh;

    if (dependency) {
        g_signal_connect(dependency, "toggled",
                         G_CALLBACK(trgp_toggle_dependent), w);
        gtk_widget_set_sensitive(w,
                                 gtk_toggle_button_get_active(dependency));
    }

    spin_refresh(priv->prefs, wd);
    priv->widgets = g_list_append(priv->widgets, wd);

    return w;
}

static void toggle_filter_trackers(GtkToggleButton * w, gpointer win)
{
    TrgStateSelector *selector =
        trg_main_window_get_state_selector(TRG_MAIN_WINDOW(win));
    trg_state_selector_set_show_trackers(selector,
                                         gtk_toggle_button_get_active(w));
}

#if TRG_WITH_GRAPH
static void toggle_graph(GtkToggleButton * w, gpointer win)
{
    if (gtk_toggle_button_get_active(w))
        trg_main_window_add_graph(TRG_MAIN_WINDOW(win), TRUE);
    else
        trg_main_window_remove_graph(TRG_MAIN_WINDOW(win));
}
#endif

static void toggle_tray_icon(GtkToggleButton * w, gpointer win)
{
    if (gtk_toggle_button_get_active(w))
        trg_main_window_add_status_icon(TRG_MAIN_WINDOW(win));
    else
        trg_main_window_remove_status_icon(TRG_MAIN_WINDOW(win));
}

static void menu_bar_toggle_filter_dirs(GtkToggleButton * w, gpointer win)
{
    TrgStateSelector *selector =
        trg_main_window_get_state_selector(TRG_MAIN_WINDOW(win));
    trg_state_selector_set_show_dirs(selector,
                                     gtk_toggle_button_get_active(w));
}

static void view_states_toggled_cb(GtkToggleButton * w, gpointer data)
{
    GtkWidget *scroll =
        gtk_widget_get_parent(GTK_WIDGET
                              (trg_main_window_get_state_selector
                               (TRG_MAIN_WINDOW(data))));
    trg_widget_set_visible(scroll, gtk_toggle_button_get_active(w));
}

static void notebook_toggled_cb(GtkToggleButton * b, gpointer data)
{
    trg_main_window_notebook_set_visible(TRG_MAIN_WINDOW(data),
                                         gtk_toggle_button_get_active(b));
}

static void
trgp_double_special_dependent(GtkWidget * widget, gpointer data)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(gtk_widget_get_toplevel
                                           (widget));
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
                                                          (widget))
                             &&
                             gtk_widget_get_sensitive
                             (priv->fullUpdateCheck)
                             &&
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
                                                          (priv->
                                                           fullUpdateCheck)));
}

static GtkWidget *trg_prefs_generalPage(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);

    GtkWidget *w, *activeOnly, *t;
    guint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Updates"));

    activeOnly = w = trgp_check_new(dlg, _("Update active torrents only"),
                                    TRG_PREFS_KEY_UPDATE_ACTIVE_ONLY,
                                    TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_wide_control(t, &row, w);

    priv->fullUpdateCheck = trgp_check_new(dlg,
                                           _
                                           ("Full update every (?) updates"),
                                           TRG_PREFS_ACTIVEONLY_FULLSYNC_ENABLED,
                                           TRG_PREFS_PROFILE,
                                           GTK_TOGGLE_BUTTON(activeOnly));
    w = trgp_spin_new(dlg, TRG_PREFS_ACTIVEONLY_FULLSYNC_EVERY, 2, INT_MAX,
                      1, TRG_PREFS_PROFILE,
                      GTK_TOGGLE_BUTTON(priv->fullUpdateCheck));
    g_signal_connect(activeOnly, "toggled",
                     G_CALLBACK(trgp_double_special_dependent), w);

    hig_workarea_add_row_w(t, &row, priv->fullUpdateCheck, w, NULL);

    w = trgp_spin_new(dlg, TRG_PREFS_KEY_UPDATE_INTERVAL, 1, INT_MAX, 1,
                      TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_row(t, &row, _("Update interval:"), w, NULL);

    w = trgp_spin_new(dlg, TRG_PREFS_KEY_MINUPDATE_INTERVAL, 1, INT_MAX, 1,
                      TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_row(t, &row, _("Minimised update interval:"), w,
                         NULL);

    w = trgp_spin_new(dlg, TRG_PREFS_KEY_SESSION_UPDATE_INTERVAL, 1,
                      INT_MAX, 1, TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_row(t, &row, _("Session update interval:"), w, NULL);

    hig_workarea_add_section_title(t, &row, _("Torrents"));

    w = trgp_check_new(dlg, _("Start paused"), TRG_PREFS_KEY_START_PAUSED,
                       TRG_PREFS_GLOBAL, NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trgp_check_new(dlg, _("Options dialog on add"),
                       TRG_PREFS_KEY_ADD_OPTIONS_DIALOG, TRG_PREFS_GLOBAL,
                       NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trgp_check_new(dlg, _("Delete local .torrent file after adding"),
                       TRG_PREFS_KEY_DELETE_LOCAL_TORRENT,
                       TRG_PREFS_GLOBAL, NULL);
    hig_workarea_add_wide_control(t, &row, w);


    return t;
}

static void profile_changed_cb(GtkWidget * w, gpointer data)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(data);
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(w));
    gint n_children = gtk_tree_model_iter_n_children(model, NULL);

    GtkTreeIter iter;
    JsonObject *profile;

    trg_pref_widget_save_all(TRG_PREFERENCES_DIALOG(data));

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(w), &iter)) {
        gtk_tree_model_get(model, &iter, 0, &profile, -1);
        trg_prefs_set_profile(priv->prefs, profile);
        trg_pref_widget_refresh_all(TRG_PREFERENCES_DIALOG(data));
        gtk_widget_set_sensitive(priv->profileDelButton, n_children > 1);
    } else {
        gtk_widget_set_sensitive(priv->profileDelButton, FALSE);
        gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
    }
}

static void
trg_prefs_profile_combo_populate(TrgPreferencesDialog * dialog,
                                 GtkComboBox * combo, TrgPrefs * prefs)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dialog);

    gint profile_id = trg_prefs_get_int(prefs, TRG_PREFS_KEY_PROFILE_ID,
                                        TRG_PREFS_GLOBAL);
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
    GList *profiles =
        json_array_get_elements(trg_prefs_get_profiles(prefs));
    GList *li;

    int i = 0;
    for (li = profiles; li; li = g_list_next(li)) {
        JsonObject *profile = json_node_get_object((JsonNode *) li->data);
        const gchar *name_value;
        GtkTreeIter iter;

        if (json_object_has_member(profile, TRG_PREFS_KEY_PROFILE_NAME)) {
            name_value = json_object_get_string_member(profile,
                                                       TRG_PREFS_KEY_PROFILE_NAME);
        } else {
            name_value = _(TRG_PROFILE_NAME_DEFAULT);
        }

        gtk_list_store_insert_with_values(store, &iter, INT_MAX, 0,
                                          profile, 1, name_value, -1);
        if (i == profile_id)
            gtk_combo_box_set_active_iter(combo, &iter);

        i++;
    }

    gtk_widget_set_sensitive(priv->profileDelButton,
                             g_list_length(profiles) > 1);

    g_list_free(profiles);
}

static GtkWidget *trg_prefs_profile_combo_new(TrgClient * tc)
{
    GtkWidget *w;
    GtkCellRenderer *r;
    GtkListStore *store =
        gtk_list_store_new(2, G_TYPE_POINTER, G_TYPE_STRING);

    w = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    r = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(w), r, FALSE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(w), r, "text", 1);

    return w;
}

static void name_changed_cb(GtkWidget * w, gpointer data)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(data);
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkComboBox *combo;

    combo = GTK_COMBO_BOX(priv->profileComboBox);
    model = gtk_combo_box_get_model(combo);

    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1,
                           gtk_entry_get_text(GTK_ENTRY(w)), -1);
    }
}

static void del_profile_cb(GtkWidget * w, gpointer data)
{
    GtkWidget *win = gtk_widget_get_toplevel(w);
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(win);
    TrgPrefs *prefs = priv->prefs;
    GtkComboBox *combo = GTK_COMBO_BOX(data);
    GtkTreeModel *profileModel = gtk_combo_box_get_model(combo);
    GtkTreeIter iter;
    JsonObject *profile;

    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        gtk_tree_model_get(profileModel, &iter, 0, &profile, -1);
        trg_prefs_del_profile(prefs, profile);
        trg_prefs_set_profile(prefs, NULL);
        gtk_list_store_remove(GTK_LIST_STORE(profileModel), &iter);
        gtk_combo_box_set_active(combo, 0);
    }
}

static void add_profile_cb(GtkWidget * w, gpointer data)
{
    GtkWidget *win = gtk_widget_get_toplevel(w);
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(win);
    GtkComboBox *combo = GTK_COMBO_BOX(data);
    GtkTreeModel *profileModel = gtk_combo_box_get_model(combo);
    GtkTreeIter iter;

    JsonObject *profile = trg_prefs_new_profile(priv->prefs);
    gtk_list_store_insert_with_values(GTK_LIST_STORE(profileModel), &iter,
                                      INT_MAX, 0, profile, 1,
                                      _(TRG_PROFILE_NAME_DEFAULT), -1);
    gtk_combo_box_set_active_iter(combo, &iter);
}

static GtkWidget *trg_prefs_openExecPage(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    GtkWidget *t;
    TrgPersistentTreeView *ptv;
    GtkListStore *model;
    trg_pref_widget_desc *wd;
    guint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Commands"));

    model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    ptv = trg_persistent_tree_view_new(priv->prefs, model,
                                       TRG_PREFS_KEY_EXEC_COMMANDS);
    trg_persistent_tree_view_set_add_select(ptv,
                                            trg_persistent_tree_view_add_column
                                            (ptv, 0,
                                             TRG_PREFS_SUBKEY_LABEL,
                                             _("Label")));
    trg_persistent_tree_view_add_column(ptv, 1,
                                        TRG_PREFS_KEY_EXEC_COMMANDS_SUBKEY_CMD,
                                        _("Command"));
    wd = trg_persistent_tree_view_get_widget_desc(ptv);
    trg_pref_widget_refresh(dlg, wd);
    priv->widgets = g_list_append(priv->widgets, wd);

    gtk_table_attach(GTK_TABLE(t), GTK_WIDGET(ptv), 1, 2, row, row + 1,
                     GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                     GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);

    return t;
}

static GtkWidget *trg_prefs_dirsPage(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    GtkWidget *t;
    TrgPersistentTreeView *ptv;
    GtkListStore *model;
    trg_pref_widget_desc *wd;
    guint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row,
                                   _("Remote Download Directories"));

    model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    ptv = trg_persistent_tree_view_new(priv->prefs, model,
                                       TRG_PREFS_KEY_DESTINATIONS);
    trg_persistent_tree_view_set_add_select(ptv,
                                            trg_persistent_tree_view_add_column
                                            (ptv, 0,
                                             TRG_PREFS_SUBKEY_LABEL,
                                             _("Label")));
    trg_persistent_tree_view_add_column(ptv, 1,
                                        TRG_PREFS_KEY_DESTINATIONS_SUBKEY_DIR,
                                        _("Directory"));
    wd = trg_persistent_tree_view_get_widget_desc(ptv);
    trg_pref_widget_refresh(dlg, wd);
    priv->widgets = g_list_append(priv->widgets, wd);

    gtk_table_attach(GTK_TABLE(t), GTK_WIDGET(ptv), 1, 2, row, row + 1,
                     GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                     GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);

    return t;
}

static GtkWidget *trg_prefs_viewPage(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);

    GtkWidget *w, *dep, *t, *tray;
    guint row = 0;
    gboolean _is_unity = is_unity();

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("View"));

    dep = w = trgp_check_new(dlg, _("State selector"),
                             TRG_PREFS_KEY_SHOW_STATE_SELECTOR,
                             TRG_PREFS_GLOBAL, NULL);
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(view_states_toggled_cb), priv->win);
    hig_workarea_add_wide_control(t, &row, w);

    w = trgp_check_new(dlg, _("Directory filters"),
                       TRG_PREFS_KEY_FILTER_DIRS, TRG_PREFS_GLOBAL,
                       GTK_TOGGLE_BUTTON(dep));
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(menu_bar_toggle_filter_dirs), priv->win);
    hig_workarea_add_wide_control(t, &row, w);

    w = trgp_check_new(dlg, _("Tracker filters"),
                       TRG_PREFS_KEY_FILTER_TRACKERS, TRG_PREFS_GLOBAL,
                       GTK_TOGGLE_BUTTON(dep));
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(toggle_filter_trackers), priv->win);
    hig_workarea_add_wide_control(t, &row, w);

    w = trgp_check_new(dlg, _("Torrent Details"),
                       TRG_PREFS_KEY_SHOW_NOTEBOOK, TRG_PREFS_GLOBAL,
                       NULL);
    g_signal_connect(G_OBJECT(w), "toggled",
                     G_CALLBACK(notebook_toggled_cb), priv->win);
    hig_workarea_add_wide_control(t, &row, w);

#if TRG_WITH_GRAPH
    w = trgp_check_new(dlg, _("Show graph"), TRG_PREFS_KEY_SHOW_GRAPH,
                       TRG_PREFS_GLOBAL, GTK_TOGGLE_BUTTON(w));
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(toggle_graph),
                     priv->win);
    hig_workarea_add_wide_control(t, &row, w);
#endif

#ifndef HAVE_LIBAPPINDICATOR
    if (!_is_unity) {
#endif
        hig_workarea_add_section_title(t, &row, _("System Tray"));

        tray = trgp_check_new(dlg, _("Show in system tray"),
                              TRG_PREFS_KEY_SYSTEM_TRAY, TRG_PREFS_GLOBAL,
                              NULL);
        g_signal_connect(G_OBJECT(tray), "toggled",
                         G_CALLBACK(toggle_tray_icon), priv->win);
        hig_workarea_add_wide_control(t, &row, tray);

        if (!_is_unity) {
            w = trgp_check_new(dlg, _("Minimise to system tray"),
                               TRG_PREFS_KEY_SYSTEM_TRAY_MINIMISE,
                               TRG_PREFS_GLOBAL, NULL);
            gtk_widget_set_sensitive(w,
                                     gtk_toggle_button_get_active
                                     (GTK_TOGGLE_BUTTON(tray)));
            g_signal_connect(G_OBJECT(tray), "toggled",
                             G_CALLBACK(toggle_active_arg_is_sensitive),
                             w);
            hig_workarea_add_wide_control(t, &row, w);
        }
#ifndef HAVE_LIBAPPINDICATOR
    }
#endif

#ifdef HAVE_LIBNOTIFY
    hig_workarea_add_section_title(t, &row, _("Notifications"));

    w = trgp_check_new(dlg, _("Torrent added notifications"),
                       TRG_PREFS_KEY_ADD_NOTIFY, TRG_PREFS_GLOBAL, NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trgp_check_new(dlg, _("Torrent complete notifications"),
                       TRG_PREFS_KEY_COMPLETE_NOTIFY, TRG_PREFS_GLOBAL,
                       NULL);
    hig_workarea_add_wide_control(t, &row, w);
#endif

    return t;
}

static GtkWidget *trg_prefs_serverPage(TrgPreferencesDialog * dlg)
{
    TrgPreferencesDialogPrivate *priv =
        TRG_PREFERENCES_DIALOG_GET_PRIVATE(dlg);
    TrgPrefs *prefs = priv->prefs;

    GtkWidget *w, *t, *frame, *frameHbox, *profileLabel;
    GtkWidget *profileButtonsHbox;
    guint row = 0;

    t = hig_workarea_create();

    /* Profile */

    priv->profileNameEntry =
        trgp_entry_new(dlg, TRG_PREFS_KEY_PROFILE_NAME, TRG_PREFS_PROFILE);

    priv->profileComboBox = trg_prefs_profile_combo_new(priv->client);
    profileLabel = gtk_label_new(_("Profile: "));

    profileButtonsHbox = trg_hbox_new(FALSE, 0);
    w = gtk_button_new_from_stock(GTK_STOCK_NEW);
    g_signal_connect(w, "clicked", G_CALLBACK(add_profile_cb),
                     priv->profileComboBox);
    gtk_box_pack_start(GTK_BOX(profileButtonsHbox), w, FALSE, FALSE, 4);

    priv->profileDelButton = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    g_signal_connect(priv->profileDelButton, "clicked",
                     G_CALLBACK(del_profile_cb), priv->profileComboBox);
    gtk_widget_set_sensitive(priv->profileDelButton, FALSE);
    gtk_box_pack_start(GTK_BOX(profileButtonsHbox), priv->profileDelButton,
                       FALSE, FALSE, 4);

    trg_prefs_profile_combo_populate(dlg,
                                     GTK_COMBO_BOX(priv->profileComboBox),
                                     prefs);
    g_signal_connect(G_OBJECT(priv->profileComboBox), "changed",
                     G_CALLBACK(profile_changed_cb), dlg);

    /* Name */

    g_signal_connect(priv->profileNameEntry, "changed",
                     G_CALLBACK(name_changed_cb), dlg);

    hig_workarea_add_row(t, &row, _("Name:"), priv->profileNameEntry,
                         NULL);

    gtk_table_attach(GTK_TABLE(t), profileButtonsHbox, 1, 2, row, row + 1,
                     GTK_EXPAND | GTK_SHRINK, 0, 0, 0);

    row++;

    hig_workarea_add_section_title(t, &row, _("Connection"));

    w = trgp_entry_new(dlg, TRG_PREFS_KEY_HOSTNAME, TRG_PREFS_PROFILE);
    hig_workarea_add_row(t, &row, _("Host:"), w, NULL);

    w = trgp_spin_new(dlg, TRG_PREFS_KEY_PORT, 1, 65535, 1,
                      TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_row(t, &row, _("Port:"), w, NULL);

    w = trgp_entry_new(dlg, TRG_PREFS_KEY_USERNAME, TRG_PREFS_PROFILE);
    hig_workarea_add_row(t, &row, _("Username:"), w, NULL);

    w = trgp_entry_new(dlg, TRG_PREFS_KEY_PASSWORD, TRG_PREFS_PROFILE);
    gtk_entry_set_visibility(GTK_ENTRY(w), FALSE);
    hig_workarea_add_row(t, &row, _("Password:"), w, NULL);

    w = trgp_check_new(dlg, _("Automatically connect"),
                       TRG_PREFS_KEY_AUTO_CONNECT, TRG_PREFS_PROFILE,
                       NULL);
    hig_workarea_add_wide_control(t, &row, w);

#ifndef CURL_NO_SSL
    w = trgp_check_new(dlg, _("SSL"), TRG_PREFS_KEY_SSL, TRG_PREFS_PROFILE,
                       NULL);
    hig_workarea_add_wide_control(t, &row, w);
#endif

    w = trgp_spin_new(dlg, TRG_PREFS_KEY_TIMEOUT, 1, 3600, 1,
                      TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_row(t, &row, _("Timeout:"), w, NULL);

    w = trgp_spin_new(dlg, TRG_PREFS_KEY_RETRIES, 0, 3600, 1,
                      TRG_PREFS_PROFILE, NULL);
    hig_workarea_add_row(t, &row, _("Retries:"), w, NULL);

    frame = gtk_frame_new(NULL);
    frameHbox = trg_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(frameHbox), profileLabel, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(frameHbox), priv->profileComboBox, FALSE,
                       FALSE, 4);
    gtk_frame_set_label_widget(GTK_FRAME(frame), frameHbox);
    gtk_container_add(GTK_CONTAINER(frame), t);

    return frame;
}

static GObject *trg_preferences_dialog_constructor(GType type,
                                                   guint
                                                   n_construct_properties,
                                                   GObjectConstructParam *
                                                   construct_params)
{
    GObject *object;
    TrgPreferencesDialogPrivate *priv;
    GtkWidget *notebook, *contentvbox;

    object = G_OBJECT_CLASS
        (trg_preferences_dialog_parent_class)->constructor(type,
                                                           n_construct_properties,
                                                           construct_params);
    priv = TRG_PREFERENCES_DIALOG_GET_PRIVATE(object);

    contentvbox = gtk_dialog_get_content_area(GTK_DIALOG(object));

    gtk_window_set_transient_for(GTK_WINDOW(object),
                                 GTK_WINDOW(priv->win));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(object), TRUE);
    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_CLOSE,
                          GTK_RESPONSE_CLOSE);
    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_OK,
                          GTK_RESPONSE_OK);

    gtk_dialog_set_default_response(GTK_DIALOG(object), GTK_RESPONSE_OK);

    gtk_window_set_title(GTK_WINDOW(object), _("Local Preferences"));
    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    g_signal_connect(G_OBJECT(object), "response",
                     G_CALLBACK(trg_preferences_response_cb), NULL);

    notebook = gtk_notebook_new();

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_serverPage(TRG_PREFERENCES_DIALOG
                                                  (object)),
                             gtk_label_new(_("Connection")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_generalPage(TRG_PREFERENCES_DIALOG
                                                   (object)),
                             gtk_label_new(_("General")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_viewPage(TRG_PREFERENCES_DIALOG
                                                (object)),
                             gtk_label_new(_("View")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_openExecPage(TRG_PREFERENCES_DIALOG
                                                    (object)),
                             gtk_label_new(_("Actions")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_prefs_dirsPage(TRG_PREFERENCES_DIALOG
                                                (object)),
                             gtk_label_new(_("Directories")));

    gtk_container_set_border_width(GTK_CONTAINER(notebook), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(contentvbox), notebook, TRUE, TRUE, 0);

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

    g_object_class_install_property(g_object_class,
                                    PROP_MAIN_WINDOW,
                                    g_param_spec_object("main-window",
                                                        "Main Window",
                                                        "Main Window",
                                                        TRG_TYPE_MAIN_WINDOW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_type_class_add_private(g_object_class,
                             sizeof(TrgPreferencesDialogPrivate));
}

GtkWidget *trg_preferences_dialog_get_instance(TrgMainWindow * win,
                                               TrgClient * client)
{
    if (!instance)
        instance =
            g_object_new(TRG_TYPE_PREFERENCES_DIALOG, "main-window", win,
                         "trg-client", client, NULL);

    return GTK_WIDGET(instance);
}
