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

#include <stdint.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-main-window.h"
#include "trg-remote-prefs-dialog.h"
#include "hig.h"
#include "util.h"
#include "requests.h"
#include "json.h"
#include "trg-json-widgets.h"
#include "session-get.h"

G_DEFINE_TYPE(TrgRemotePrefsDialog, trg_remote_prefs_dialog,
        GTK_TYPE_DIALOG)
#define TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_REMOTE_PREFS_DIALOG, TrgRemotePrefsDialogPrivate))
enum {
    PROP_0, PROP_PARENT, PROP_CLIENT
};

typedef struct _TrgRemotePrefsDialogPrivate TrgRemotePrefsDialogPrivate;

struct _TrgRemotePrefsDialogPrivate {
    TrgClient *client;
    TrgMainWindow *parent;

    GList *widgets;
    GtkWidget *encryption_combo;
    GtkWidget *port_test_label, *port_test_button;
    GtkWidget *blocklist_update_button, *blocklist_check;
};

static GObject *instance = NULL;

static void update_session(GtkDialog * dlg) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(dlg);

    JsonNode *request = session_set();
    JsonObject *args = node_get_arguments(request);
    gchar *encryption;

    /* Connection */

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(priv->encryption_combo))) {
    case 0:
        encryption = "required";
        break;
    case 2:
        encryption = "tolerated";
        break;
    default:
        encryption = "preferred";
        break;
    }

    json_object_set_string_member(args, SGET_ENCRYPTION, encryption);

    trg_json_widgets_save(priv->widgets, args);

    dispatch_async(priv->client, request, on_session_set, priv->parent);
}

static void trg_remote_prefs_response_cb(GtkDialog * dlg, gint res_id,
        gpointer data G_GNUC_UNUSED) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(dlg);

    if (res_id == GTK_RESPONSE_OK)
        update_session(dlg);

    trg_json_widget_desc_list_free(priv->widgets);

    gtk_widget_destroy(GTK_WIDGET(dlg));
    instance = NULL;
}

static void trg_remote_prefs_dialog_get_property(GObject * object,
        guint property_id, GValue * value, GParamSpec * pspec) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_PARENT:
        g_value_set_object(value, priv->parent);
        break;
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_remote_prefs_dialog_set_property(GObject * object,
        guint property_id, const GValue * value, GParamSpec * pspec) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_PARENT:
        priv->parent = g_value_get_object(value);
        break;
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static GtkWidget *trg_rprefs_limitsPage(TrgRemotePrefsDialog * win,
        JsonObject * json) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(win);
    GtkWidget *w, *tb, *t;
    gint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Bandwidth"));

    tb = trg_json_widget_check_new(
            &priv->widgets, json, SGET_SPEED_LIMIT_DOWN_ENABLED, _("Limit download speed (KB/s)"), NULL);
    w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_SPEED_LIMIT_DOWN, tb, 0, INT_MAX, 1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    tb = trg_json_widget_check_new(&priv->widgets, json, SGET_SPEED_LIMIT_UP_ENABLED, _("Limit upload speed (KB/s)"), NULL);
    w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_SPEED_LIMIT_UP, tb, 0, INT_MAX, 1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    hig_workarea_add_section_title(t, &row, _("Seeding"));

    tb = trg_json_widget_check_new(
            &priv->widgets, json, SGET_SEED_RATIO_LIMITED, _("Seed ratio limit"), NULL);
    w = trg_json_widget_spin_new_double(&priv->widgets, json, SGET_SEED_RATIO_LIMIT, tb, 0, INT_MAX, 0.1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    if (json_object_has_member(json, SGET_DOWNLOAD_QUEUE_ENABLED)) {
        hig_workarea_add_section_title(t, &row, _("Queues"));

        tb = trg_json_widget_check_new(
                &priv->widgets, json, SGET_DOWNLOAD_QUEUE_ENABLED, _("Download queue size"), NULL);
        w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_DOWNLOAD_QUEUE_SIZE, tb, 0, INT_MAX, 1);
        hig_workarea_add_row_w(t, &row, tb, w, NULL);

        tb = trg_json_widget_check_new(
                &priv->widgets, json, SGET_SEED_QUEUE_ENABLED, _("Seed queue size"), NULL);
        w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_SEED_QUEUE_SIZE, tb, 0, INT_MAX, 1);
        hig_workarea_add_row_w(t, &row, tb, w, NULL);

        tb = trg_json_widget_check_new(
                &priv->widgets, json, SGET_QUEUE_STALLED_ENABLED, _("Ignore stalled (minutes)"), NULL);
        w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_QUEUE_STALLED_MINUTES, tb, 0, INT_MAX, 1);
        hig_workarea_add_row_w(t, &row, tb, w, NULL);
    }

    hig_workarea_add_section_title(t, &row, _("Peers"));

    w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_PEER_LIMIT_GLOBAL, NULL, 0, INT_MAX, 5);
    hig_workarea_add_row(t, &row, _("Global peer limit"), w, w);

    w = trg_json_widget_spin_new_int(&priv->widgets, json, SGET_PEER_LIMIT_PER_TORRENT, NULL, 0, INT_MAX, 5);
    hig_workarea_add_row(t, &row, _("Per torrent peer limit"), w, w);

    return t;
}

static gboolean on_port_tested(gpointer data) {
	trg_response *response = (trg_response*)data;
    if (TRG_IS_REMOTE_PREFS_DIALOG(response->cb_data)) {
        TrgRemotePrefsDialogPrivate *priv =
                TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(response->cb_data);

        gtk_button_set_label(GTK_BUTTON(priv->port_test_button), _("Retest"));
        gtk_widget_set_sensitive(priv->port_test_button, TRUE);

        if (response->status == CURLE_OK) {
            gboolean isOpen = json_object_get_boolean_member(
                    get_arguments(response->obj), "port-is-open");
            if (isOpen)
                gtk_label_set_markup(
                        GTK_LABEL(priv->port_test_label),
                        _
                        ("Port is <span font_weight=\"bold\" fgcolor=\"darkgreen\">open</span>"));
            else
                gtk_label_set_markup(
                        GTK_LABEL(priv->port_test_label),
                        _
                        ("Port is <span font_weight=\"bold\" fgcolor=\"red\">closed</span>"));
        } else {
            trg_error_dialog(GTK_WINDOW(data), response);
        }
    }

    trg_response_free(response);
    return FALSE;
}

static void port_test_cb(GtkButton * b, gpointer data) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(data);
    JsonNode *req = port_test();

    gtk_label_set_text(GTK_LABEL(priv->port_test_label), _("Port test"));
    gtk_button_set_label(b, _("Testing..."));
    gtk_widget_set_sensitive(GTK_WIDGET(b), FALSE);

    dispatch_async(priv->client, req, on_port_tested, data);
}

static gboolean on_blocklist_updated(gpointer data) {
	trg_response *response = (trg_response*)data;
    if (TRG_IS_REMOTE_PREFS_DIALOG(response->cb_data)) {
        TrgRemotePrefsDialogPrivate *priv =
                TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(response->cb_data);

        gtk_widget_set_sensitive(priv->blocklist_update_button, TRUE);
        gtk_button_set_label(GTK_BUTTON(priv->blocklist_update_button),
                "Update");

        if (response->status == CURLE_OK) {
            JsonObject *args = get_arguments(response->obj);
            gchar *labelText = g_strdup_printf(_("Blocklist (%ld entries)"),
                    json_object_get_int_member(args, SGET_BLOCKLIST_SIZE));
            gtk_button_set_label(GTK_BUTTON(priv->blocklist_check), labelText);
            g_free(labelText);
        } else {
            trg_error_dialog(GTK_WINDOW(response->cb_data), response);
        }
    }

    trg_response_free(response);

    return FALSE;
}

static gboolean update_blocklist_cb(GtkButton * b, gpointer data) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(data);
    JsonNode *req = blocklist_update();

    gtk_widget_set_sensitive(GTK_WIDGET(b), FALSE);
    gtk_button_set_label(b, _("Updating..."));

    dispatch_async(priv->client, req, on_blocklist_updated, data);

    return FALSE;
}

static GtkWidget *trg_rprefs_connPage(TrgRemotePrefsDialog * win,
        JsonObject * s) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(win);

    GtkWidget *w, *tb, *t;
    const gchar *stringValue;
    gint row = 0;

    t = hig_workarea_create();

    w = priv->encryption_combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("Required"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("Preferred"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(w), _("Tolerated"));
    stringValue = session_get_encryption(s);
    if (g_strcmp0(stringValue, "required") == 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
    } else if (g_strcmp0(stringValue, "tolerated") == 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(w), 2);
    } else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(w), 1);
    }
    hig_workarea_add_row(t, &row, _("Encryption"), w, NULL);

    w = trg_json_widget_spin_new_int(&priv->widgets, s, SGET_PEER_PORT, NULL, 0, 65535, 1);
    hig_workarea_add_row(t, &row, _("Peer port"), w, w);

    w = priv->port_test_label = gtk_label_new(_("Port test"));
    w = priv->port_test_button = gtk_button_new_with_label(_("Test"));
    g_signal_connect(w, "clicked", G_CALLBACK(port_test_cb), win);
    hig_workarea_add_row_w(t, &row, priv->port_test_label, w, NULL);

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_PEER_PORT_RANDOM_ON_START, _("Random peer port on start"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_PORT_FORWARDING_ENABLED, _("Peer port forwarding"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_PEX_ENABLED, _("Peer exchange (PEX)"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_LPD_ENABLED, _("Local peer discovery"), NULL);
        hig_workarea_add_wide_control(t, &row, w);

    stringValue = g_strdup_printf(_("Blocklist (%ld entries)"),
            session_get_blocklist_size(s));
    tb = priv->blocklist_check = trg_json_widget_check_new(&priv->widgets, s, SGET_BLOCKLIST_ENABLED, stringValue, NULL);
    g_free((gchar *) stringValue);

    w = priv->blocklist_update_button = gtk_button_new_with_label(_("Update"));
    g_signal_connect(G_OBJECT(w), "clicked",
            G_CALLBACK(update_blocklist_cb), win);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    //GtkWidget *trg_json_widget_entry_new(GList **wl, JsonObject *obj, gchar *key, GtkWidget *toggleDep);
    stringValue = session_get_blocklist_url(s);
    if (stringValue) {
        w = trg_json_widget_entry_new(&priv->widgets, s, SGET_BLOCKLIST_URL, NULL);
        hig_workarea_add_row(t, &row, _("Blocklist URL:"), w, NULL);
    }

    return t;
}

static GtkWidget *trg_rprefs_generalPage(TrgRemotePrefsDialog * win,
        JsonObject * s) {
    TrgRemotePrefsDialogPrivate *priv =
            TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(win);

    GtkWidget *w, *tb, *t;
    gint row = 0;
    gint64 cache_size_mb;

    t = hig_workarea_create();

    w = trg_json_widget_entry_new(&priv->widgets, s, SGET_DOWNLOAD_DIR, NULL);
    hig_workarea_add_row(t, &row, _("Download directory"), w, NULL);

    tb = trg_json_widget_check_new(&priv->widgets, s, SGET_INCOMPLETE_DIR_ENABLED, _("Incomplete download dir"), NULL);
    w = trg_json_widget_entry_new(&priv->widgets, s, SGET_INCOMPLETE_DIR, tb);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    tb = trg_json_widget_check_new(&priv->widgets, s, SGET_SCRIPT_TORRENT_DONE_ENABLED, _("Torrent done script"), NULL);
    w = trg_json_widget_entry_new(&priv->widgets, s, SGET_SCRIPT_TORRENT_DONE_FILENAME, tb);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    cache_size_mb = session_get_cache_size_mb(s);
    if (cache_size_mb >= 0) {
        w = trg_json_widget_spin_new_int(&priv->widgets, s, SGET_CACHE_SIZE_MB, NULL, 0,
                INT_MAX, 1);
        hig_workarea_add_row(t, &row, _("Cache size (MB)"), w, w);
    }

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_RENAME_PARTIAL_FILES, _("Rename partial files"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_TRASH_ORIGINAL_TORRENT_FILES, _
            ("Trash original torrent files"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = trg_json_widget_check_new(&priv->widgets, s, SGET_START_ADDED_TORRENTS,
            _("Start added torrents"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    return t;
}

static GObject *trg_remote_prefs_dialog_constructor(GType type,
        guint n_construct_properties, GObjectConstructParam * construct_params) {
    GObject *object;
    TrgRemotePrefsDialogPrivate *priv;
    JsonObject *session;
    GtkWidget *notebook, *contentvbox;

    object = G_OBJECT_CLASS
    (trg_remote_prefs_dialog_parent_class)->constructor(type,
            n_construct_properties, construct_params);
    priv = TRG_REMOTE_PREFS_DIALOG_GET_PRIVATE(object);
    session = trg_client_get_session(priv->client);

    contentvbox = gtk_dialog_get_content_area(GTK_DIALOG(object));

    gtk_window_set_title(GTK_WINDOW(object), _("Remote Preferences"));
    gtk_window_set_transient_for(GTK_WINDOW(object), GTK_WINDOW(priv->parent));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(object), TRUE);

    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_CLOSE,
            GTK_RESPONSE_CLOSE);
    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_OK, GTK_RESPONSE_OK);

    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(object), GTK_RESPONSE_OK);

    gtk_dialog_set_alternative_button_order(GTK_DIALOG(object),
            GTK_RESPONSE_OK, GTK_RESPONSE_CLOSE, -1);

    g_signal_connect(G_OBJECT(object),
            "response",
            G_CALLBACK(trg_remote_prefs_response_cb), NULL);

    notebook = gtk_notebook_new();

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            trg_rprefs_generalPage(TRG_REMOTE_PREFS_DIALOG(object), session),
            gtk_label_new(_("General")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            trg_rprefs_connPage(TRG_REMOTE_PREFS_DIALOG(object), session),
            gtk_label_new(_("Connections")));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            trg_rprefs_limitsPage(TRG_REMOTE_PREFS_DIALOG(object), session),
            gtk_label_new(_("Limits")));

    gtk_container_set_border_width(GTK_CONTAINER(notebook), GUI_PAD);

    gtk_box_pack_start(GTK_BOX(contentvbox), notebook, TRUE, TRUE,
            0);

    return object;
}

static void trg_remote_prefs_dialog_class_init(
        TrgRemotePrefsDialogClass * klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgRemotePrefsDialogPrivate));

    object_class->constructor = trg_remote_prefs_dialog_constructor;
    object_class->get_property = trg_remote_prefs_dialog_get_property;
    object_class->set_property = trg_remote_prefs_dialog_set_property;

    g_object_class_install_property(
            object_class,
            PROP_CLIENT,
            g_param_spec_pointer(
                    "trg-client",
                    "TClient",
                    "Client",
                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
                            | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                            | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
            object_class,
            PROP_PARENT,
            g_param_spec_object(
                    "parent-window",
                    "Parent window",
                    "Parent window",
                    TRG_TYPE_MAIN_WINDOW,
                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
                            | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                            | G_PARAM_STATIC_BLURB));
}

static void trg_remote_prefs_dialog_init(TrgRemotePrefsDialog * self G_GNUC_UNUSED) {
}

TrgRemotePrefsDialog *trg_remote_prefs_dialog_get_instance(
        TrgMainWindow * parent, TrgClient * client) {
    if (instance == NULL) {
        instance = g_object_new(TRG_TYPE_REMOTE_PREFS_DIALOG, "parent-window",
                parent, "trg-client", client, NULL);
    }

    return TRG_REMOTE_PREFS_DIALOG(instance);
}
