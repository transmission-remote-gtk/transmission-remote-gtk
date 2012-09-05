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
#include <json-glib/json-glib.h>

#include "util.h"
#include "torrent.h"
#include "json.h"
#include "trg-client.h"
#include "trg-json-widgets.h"
#include "requests.h"
#include "protocol-constants.h"
#include "trg-peers-model.h"
#include "trg-peers-tree-view.h"
#include "trg-files-model.h"
#include "trg-files-tree-view.h"
#include "trg-trackers-model.h"
#include "trg-trackers-tree-view.h"
#include "trg-torrent-model.h"
#include "trg-torrent-tree-view.h"
#include "trg-torrent-props-dialog.h"
#include "trg-main-window.h"
#include "hig.h"

/* Pretty similar to remote preferences, also using the widget creation functions
 * in trg-json-widgets.c. The torrent tree view is passed into here, which this
 * gets the selection from. If there are multiple selections, use the first to
 * populate the fields.
 *
 * Build the JSON array of torrent IDs when the dialog is created, in case the
 * selection changes before clicking OK.
 *
 * When the user clicks OK, use trg-json-widgets to populate an object with the
 * values and then send it with the IDs.
 */

G_DEFINE_TYPE(TrgTorrentPropsDialog, trg_torrent_props_dialog,
              GTK_TYPE_DIALOG)
enum {
    PROP_0, PROP_TREEVIEW, PROP_TORRENT_MODEL, PROP_PARENT_WINDOW,
    PROP_CLIENT
};

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_PROPS_DIALOG, TrgTorrentPropsDialogPrivate))
typedef struct _TrgTorrentPropsDialogPrivate TrgTorrentPropsDialogPrivate;

struct _TrgTorrentPropsDialogPrivate {
    TrgTorrentTreeView *tv;
    TrgTorrentModel *torrentModel;
    TrgClient *client;
    TrgMainWindow *parent;
    JsonArray *targetIds;

    GList *widgets;

    GtkWidget *bandwidthPriorityCombo, *seedRatioMode;

    TrgPeersTreeView *peersTv;
    TrgPeersModel *peersModel;
    TrgTrackersTreeView *trackersTv;
    TrgTrackersModel *trackersModel;
    TrgFilesTreeView *filesTv;
    TrgFilesModel *filesModel;
    JsonObject *lastJson;

    GtkWidget *size_lb;
    GtkWidget *have_lb;
    GtkWidget *dl_lb;
    GtkWidget *ul_lb;
    GtkWidget *state_lb;
    GtkWidget *date_started_lb;
    GtkWidget *eta_lb;
    GtkWidget *last_activity_lb;
    GtkWidget *error_lb;
    GtkWidget *destination_lb;
    GtkWidget *hash_lb;
    GtkWidget *privacy_lb;
    GtkWidget *origin_lb;
    GtkTextBuffer *comment_buffer;
    gboolean show_details;
};

static void trg_torrent_props_dialog_set_property(GObject * object,
                                                  guint prop_id,
                                                  const GValue * value,
                                                  GParamSpec *
                                                  pspec G_GNUC_UNUSED)
{
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_PARENT_WINDOW:
        priv->parent = g_value_get_object(value);
        break;
    case PROP_TREEVIEW:
        priv->tv = g_value_get_object(value);
        break;
    case PROP_TORRENT_MODEL:
        priv->torrentModel = g_value_get_object(value);
        break;
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    }
}

static void trg_torrent_props_dialog_get_property(GObject * object,
                                                  guint prop_id,
                                                  GValue * value,
                                                  GParamSpec *
                                                  pspec G_GNUC_UNUSED)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

static void trg_torrent_props_response_cb(GtkDialog * dialog, gint res_id,
                                          gpointer data G_GNUC_UNUSED)
{
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(dialog);
    JsonNode *request;
    JsonObject *args;

    if (priv->peersTv)
        trg_tree_view_persist(TRG_TREE_VIEW(priv->peersTv),
                              TRG_TREE_VIEW_PERSIST_SORT |
                              TRG_TREE_VIEW_PERSIST_LAYOUT);
    if (priv->filesTv)
        trg_tree_view_persist(TRG_TREE_VIEW(priv->filesTv),
                              TRG_TREE_VIEW_PERSIST_SORT |
                              TRG_TREE_VIEW_PERSIST_LAYOUT);

    if (priv->trackersTv)
        trg_tree_view_persist(TRG_TREE_VIEW(priv->trackersTv),
                              TRG_TREE_VIEW_PERSIST_SORT |
                              TRG_TREE_VIEW_PERSIST_LAYOUT);

    if (res_id != GTK_RESPONSE_OK) {
        gtk_widget_destroy(GTK_WIDGET(dialog));
        json_array_unref(priv->targetIds);
        return;
    }

    request = torrent_set(priv->targetIds);
    args = node_get_arguments(request);

    json_object_set_int_member(args, FIELD_SEED_RATIO_MODE,
                               gtk_combo_box_get_active(GTK_COMBO_BOX
                                                        (priv->seedRatioMode)));
    json_object_set_int_member(args, FIELD_BANDWIDTH_PRIORITY,
                               gtk_combo_box_get_active(GTK_COMBO_BOX
                                                        (priv->bandwidthPriorityCombo))
                               - 1);

    trg_json_widgets_save(priv->widgets, args);
    trg_json_widget_desc_list_free(priv->widgets);

    dispatch_async(priv->client, request, on_generic_interactive_action,
                   priv->parent);

    if (priv->show_details) {
        TrgPrefs *prefs = trg_client_get_prefs(priv->client);
        gint width, height;
        gtk_window_get_size(GTK_WINDOW(dialog), &width, &height);
        trg_prefs_set_int(prefs, "dialog-width", width, TRG_PREFS_GLOBAL);
        trg_prefs_set_int(prefs, "dialog-height", height,
                          TRG_PREFS_GLOBAL);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void seed_ratio_mode_changed_cb(GtkWidget * w, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_combo_box_get_active(GTK_COMBO_BOX(w)) ==
                             1 ? TRUE : FALSE);
}

static GtkWidget *info_page_new(TrgTorrentPropsDialog * dialog)
{
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(dialog);
    guint row = 0;
    GtkTextBuffer *b;
    GtkWidget *l, *w, *fr, *sw;
    GtkWidget *t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Activity"));

    /* size */
    l = priv->size_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Torrent size:"), l, NULL);

    /* have */
    l = priv->have_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Have:"), l, NULL);

    /* downloaded */
    l = priv->dl_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Downloaded:"), l, NULL);

    /* uploaded */
    l = priv->ul_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Uploaded:"), l, NULL);

    /* state */
    l = priv->state_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("State:"), l, NULL);

    /* running for */
    l = priv->date_started_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Running time:"), l, NULL);

    /* eta */
    l = priv->eta_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Remaining time:"), l, NULL);

    /* last activity */
    l = priv->last_activity_lb = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Last activity:"), l, NULL);

    /* error */
    l = g_object_new(GTK_TYPE_LABEL, "selectable", TRUE, "ellipsize",
                     PANGO_ELLIPSIZE_END, NULL);
    hig_workarea_add_row(t, &row, _("Error:"), l, NULL);
    priv->error_lb = l;

    hig_workarea_add_section_divider(t, &row);
    hig_workarea_add_section_title(t, &row, _("Details"));

    /* destination */
    l = g_object_new(GTK_TYPE_LABEL, "selectable", TRUE, "ellipsize",
                     PANGO_ELLIPSIZE_END, NULL);
    hig_workarea_add_row(t, &row, _("Location:"), l, NULL);
    priv->destination_lb = l;

    /* hash */
    l = g_object_new(GTK_TYPE_LABEL, "selectable", TRUE, "ellipsize",
                     PANGO_ELLIPSIZE_END, NULL);
    hig_workarea_add_row(t, &row, _("Hash:"), l, NULL);
    priv->hash_lb = l;

    /* privacy */
    l = gtk_label_new(NULL);
    gtk_label_set_single_line_mode(GTK_LABEL(l), TRUE);
    hig_workarea_add_row(t, &row, _("Privacy:"), l, NULL);
    priv->privacy_lb = l;

    /* origins */
    l = g_object_new(GTK_TYPE_LABEL, "selectable", TRUE, "ellipsize",
                     PANGO_ELLIPSIZE_END, NULL);
    hig_workarea_add_row(t, &row, _("Origin:"), l, NULL);
    priv->origin_lb = l;

    /* comment */
    b = priv->comment_buffer = gtk_text_buffer_new(NULL);
    w = gtk_text_view_new_with_buffer(b);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(w), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(w), FALSE);
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sw, 350, 36);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), w);
    fr = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(fr), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(fr), sw);
    w = hig_workarea_add_tall_row(t, &row, _("Comment:"), fr, NULL);
    gtk_misc_set_alignment(GTK_MISC(w), 0.0f, 0.0f);

    hig_workarea_add_section_divider(t, &row);
    return t;
}

static void info_page_update(TrgTorrentPropsDialog * dialog,
                             JsonObject * t,
                             TrgTorrentModel * torrentModel,
                             GtkTreeIter * iter)
{
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(dialog);
    GtkTreeModel *model = GTK_TREE_MODEL(torrentModel);
    gint64 sizeWhenDone, haveValid, downloaded, uploaded, percentDone, eta,
        activityDate, error;
    gchar *statusString;
    guint flags;
    const gchar *str;

    char buf[512];

    gtk_tree_model_get(model, iter,
                       TORRENT_COLUMN_SIZEWHENDONE, &sizeWhenDone,
                       TORRENT_COLUMN_UPLOADED, &uploaded,
                       TORRENT_COLUMN_DOWNLOADED, &downloaded,
                       TORRENT_COLUMN_HAVE_VALID, &haveValid,
                       TORRENT_COLUMN_PERCENTDONE, &percentDone,
                       TORRENT_COLUMN_ETA, &eta,
                       TORRENT_COLUMN_LASTACTIVE, &activityDate,
                       TORRENT_COLUMN_STATUS, &statusString,
                       TORRENT_COLUMN_FLAGS, &flags,
                       TORRENT_COLUMN_ERROR, &error, -1);

    if (torrent_get_is_private(t))
        str = _("Private to this tracker -- DHT and PEX disabled");
    else
        str = _("Public torrent");

    gtk_label_set_text(GTK_LABEL(priv->privacy_lb), str);

    {
        const gchar *creator = torrent_get_creator(t);
        gint64 dateCreated = torrent_get_date_created(t);
        gchar *dateStr = epoch_to_string(dateCreated);

        if (!creator || strlen(creator) <= 0)
            g_snprintf(buf, sizeof(buf), _("Created on %1$s"), dateStr);
        else
            g_snprintf(buf, sizeof(buf), _("Created by %1$s on %2$s"),
                       creator, dateStr);

        g_free(dateStr);
        gtk_label_set_text(GTK_LABEL(priv->origin_lb), buf);
    }

    gtk_text_buffer_set_text(priv->comment_buffer, torrent_get_comment(t),
                             -1);
    gtk_label_set_text(GTK_LABEL(priv->destination_lb),
                       torrent_get_download_dir(t));

    gtk_label_set_text(GTK_LABEL(priv->state_lb), statusString);
    g_free(statusString);

    {
        gchar *addedStr = epoch_to_string(torrent_get_added_date(t));
        gtk_label_set_text(GTK_LABEL(priv->date_started_lb), addedStr);
        g_free(addedStr);
    }

    /* eta */

    if (eta > 0) {
        tr_strltime_long(buf, eta, sizeof(buf));
        gtk_label_set_text(GTK_LABEL(priv->eta_lb), buf);
    } else {
        gtk_label_set_text(GTK_LABEL(priv->eta_lb), "");
    }

    gtk_label_set_text(GTK_LABEL(priv->hash_lb), torrent_get_hash(t));
    gtk_label_set_text(GTK_LABEL(priv->error_lb),
                       error ? torrent_get_errorstr(t) : _("No errors"));

    if (flags & TORRENT_FLAG_ACTIVE) {
        gtk_label_set_text(GTK_LABEL(priv->last_activity_lb),
                           _("Active now"));
    } else {
        gchar *activityStr = epoch_to_string(activityDate);
        gtk_label_set_text(GTK_LABEL(priv->last_activity_lb), activityStr);
        g_free(activityStr);
    }

    tr_strlsize(buf, sizeWhenDone, sizeof(buf));
    gtk_label_set_text(GTK_LABEL(priv->size_lb), buf);

    tr_strlsize(buf, downloaded, sizeof(buf));
    gtk_label_set_text(GTK_LABEL(priv->dl_lb), buf);

    tr_strlsize(buf, uploaded, sizeof(buf));
    gtk_label_set_text(GTK_LABEL(priv->ul_lb), buf);

    tr_strlsize(buf, haveValid, sizeof(buf));
    gtk_label_set_text(GTK_LABEL(priv->have_lb), buf);
}

static GtkWidget *trg_props_limits_page_new(TrgTorrentPropsDialog * win,
                                            JsonObject * json)
{
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(win);
    GtkWidget *w, *tb, *t;
    guint row = 0;

    t = hig_workarea_create();

    hig_workarea_add_section_title(t, &row, _("Bandwidth"));

    w = trg_json_widget_check_new(&priv->widgets, json,
                                  FIELD_HONORS_SESSION_LIMITS,
                                  _("Honor global limits"), NULL);
    hig_workarea_add_wide_control(t, &row, w);

    w = priv->bandwidthPriorityCombo = gtr_combo_box_new_enum(_("Low"), 0,
                                                              _("Normal"),
                                                              1, _("High"),
                                                              2, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(w),
                             torrent_get_bandwidth_priority(json) + 1);

    hig_workarea_add_row(t, &row, _("Torrent priority:"), w, NULL);

    if (json_object_has_member(json, FIELD_QUEUE_POSITION)) {
        w = trg_json_widget_spin_new(&priv->widgets, json,
                                     FIELD_QUEUE_POSITION, NULL, 0,
                                     INT_MAX, 1);
        hig_workarea_add_row(t, &row, _("Queue Position:"), w, w);
    }

    tb = trg_json_widget_check_new(&priv->widgets, json,
                                   FIELD_DOWNLOAD_LIMITED,
                                   _("Limit download speed (KiB/s)"),
                                   NULL);
    w = trg_json_widget_spin_new(&priv->widgets, json,
                                 FIELD_DOWNLOAD_LIMIT, tb, 0, INT_MAX, 1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    tb = trg_json_widget_check_new(&priv->widgets, json,
                                   FIELD_UPLOAD_LIMITED,
                                   _("Limit upload speed (KiB/s)"), NULL);
    w = trg_json_widget_spin_new(&priv->widgets, json, FIELD_UPLOAD_LIMIT,
                                 tb, 0, INT_MAX, 1);
    hig_workarea_add_row_w(t, &row, tb, w, NULL);

    hig_workarea_add_section_title(t, &row, _("Seeding"));

    w = priv->seedRatioMode =
        gtr_combo_box_new_enum(_("Use global settings"), 0,
                               _("Stop seeding at ratio"), 1,
                               _("Seed regardless of ratio"), 2, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(w),
                             torrent_get_seed_ratio_mode(json));
    hig_workarea_add_row(t, &row, _("Seed ratio mode:"), w, NULL);

    w = trg_json_widget_spin_new(&priv->widgets, json,
                                 FIELD_SEED_RATIO_LIMIT, NULL, 0, INT_MAX,
                                 0.2);
    seed_ratio_mode_changed_cb(priv->seedRatioMode, w);
    g_signal_connect(G_OBJECT(priv->seedRatioMode), "changed",
                     G_CALLBACK(seed_ratio_mode_changed_cb), w);
    hig_workarea_add_row(t, &row, _("Seed ratio limit:"), w, w);

    hig_workarea_add_section_title(t, &row, _("Peers"));

    w = trg_json_widget_spin_new(&priv->widgets, json, FIELD_PEER_LIMIT,
                                 NULL, 0, INT_MAX, 5);
    hig_workarea_add_row(t, &row, _("Peer limit:"), w, w);

    return t;
}

static void models_updated(TrgTorrentModel * model, gpointer data)
{
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(data);
    GHashTable *ht = get_torrent_table(model);
    gint64 serial = trg_client_get_serial(priv->client);
    JsonObject *t = NULL;
    GtkTreeIter iter;
    gboolean exists = get_torrent_data(ht,
                                       json_array_get_int_element(priv->
                                                                  targetIds,
                                                                  0), &t,
                                       &iter);

    if (exists && priv->lastJson != t) {
        trg_files_model_update(priv->filesModel,
                               GTK_TREE_VIEW(priv->filesTv), serial, t,
                               TORRENT_GET_MODE_UPDATE);
        trg_peers_model_update(priv->peersModel,
                               TRG_TREE_VIEW(priv->peersTv), serial, t,
                               TORRENT_GET_MODE_UPDATE);
        trg_trackers_model_update(priv->trackersModel, serial, t,
                                  TORRENT_GET_MODE_UPDATE);
        info_page_update(TRG_TORRENT_PROPS_DIALOG(data), t, model, &iter);
    }

    gtk_widget_set_sensitive(GTK_WIDGET(priv->filesTv), exists);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->trackersTv), exists);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->peersTv), exists);

    priv->lastJson = t;
}

static GObject *trg_torrent_props_dialog_constructor(GType type,
                                                     guint
                                                     n_construct_properties,
                                                     GObjectConstructParam
                                                     * construct_params)
{
    GObject *object = G_OBJECT_CLASS
        (trg_torrent_props_dialog_parent_class)->constructor(type,
                                                             n_construct_properties,
                                                             construct_params);

    TrgTorrentPropsDialog *propsDialog = TRG_TORRENT_PROPS_DIALOG(object);
    GtkWindow *window = GTK_WINDOW(object);
    TrgTorrentPropsDialogPrivate *priv = GET_PRIVATE(object);

    GtkTreeSelection *selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->tv));
    gint rowCount = gtk_tree_selection_count_selected_rows(selection);
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);
    priv->show_details = trg_prefs_get_int(prefs, TRG_PREFS_KEY_STYLE,
                                           TRG_PREFS_GLOBAL) !=
        TRG_STYLE_CLASSIC && rowCount == 1;

    gint64 width, height;

    JsonObject *json;
    GtkTreeIter iter;
    GtkWidget *notebook, *contentvbox;

    get_torrent_data(trg_client_get_torrent_table(priv->client),
                     trg_mw_get_selected_torrent_id(priv->parent), &json,
                     &iter);
    priv->targetIds = build_json_id_array(priv->tv);

    if (rowCount > 1) {
        gchar *windowTitle =
            g_strdup_printf(_("Multiple (%d) torrent properties"),
                            rowCount);
        gtk_window_set_title(window, windowTitle);
        g_free(windowTitle);
    } else if (rowCount == 1) {
        gtk_window_set_title(window, torrent_get_name(json));
    }

    gtk_window_set_transient_for(window, GTK_WINDOW(priv->parent));
    gtk_window_set_destroy_with_parent(window, TRUE);

    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_CLOSE,
                          GTK_RESPONSE_CLOSE);
    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_OK,
                          GTK_RESPONSE_OK);

    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(object), GTK_RESPONSE_OK);

    g_signal_connect(G_OBJECT(object), "response",
                     G_CALLBACK(trg_torrent_props_response_cb), NULL);

    notebook = gtk_notebook_new();

    if (priv->show_details) {
        gint64 serial = trg_client_get_serial(priv->client);

        /* Information */

        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 info_page_new(propsDialog),
                                 gtk_label_new(_("Information")));
        info_page_update(propsDialog, json, priv->torrentModel, &iter);

        /* Files */

        priv->filesModel = trg_files_model_new();
        priv->filesTv =
            trg_files_tree_view_new(priv->filesModel, priv->parent,
                                    priv->client,
                                    "TrgFilesTreeView-dialog");
        trg_files_model_update(priv->filesModel,
                               GTK_TREE_VIEW(priv->filesTv), serial, json,
                               TORRENT_GET_MODE_FIRST);
        gtk_widget_set_sensitive(GTK_WIDGET(priv->filesTv), TRUE);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 my_scrolledwin_new(GTK_WIDGET
                                                    (priv->filesTv)),
                                 gtk_label_new(_("Files")));

        /* Peers */

        priv->peersModel = trg_peers_model_new();
        priv->peersTv = trg_peers_tree_view_new(prefs, priv->peersModel,
                                                "TrgPeersTreeView-dialog");
        trg_peers_model_update(priv->peersModel,
                               TRG_TREE_VIEW(priv->peersTv), serial, json,
                               TORRENT_GET_MODE_FIRST);
        gtk_widget_set_sensitive(GTK_WIDGET(priv->peersTv), TRUE);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 my_scrolledwin_new(GTK_WIDGET
                                                    (priv->peersTv)),
                                 gtk_label_new(_("Peers")));

        /* Trackers */

        priv->trackersModel = trg_trackers_model_new();
        priv->trackersTv = trg_trackers_tree_view_new(priv->trackersModel,
                                                      priv->client,
                                                      priv->parent,
                                                      "TrgTrackersTreeView-dialog");
        trg_trackers_tree_view_new_connection(priv->trackersTv,
                                              priv->client);
        trg_trackers_model_update(priv->trackersModel, serial, json,
                                  TORRENT_GET_MODE_FIRST);
        gtk_widget_set_sensitive(GTK_WIDGET(priv->trackersTv), TRUE);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 my_scrolledwin_new(GTK_WIDGET
                                                    (priv->trackersTv)),
                                 gtk_label_new(_("Trackers")));

        g_object_unref(priv->trackersModel);
        g_object_unref(priv->filesModel);
        g_object_unref(priv->peersModel);

        g_signal_connect_object(priv->torrentModel, "update", G_CALLBACK
                                (models_updated), object, G_CONNECT_AFTER);

        priv->lastJson = json;
    }

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             trg_props_limits_page_new(propsDialog, json),
                             gtk_label_new(_("Limits")));

    gtk_container_set_border_width(GTK_CONTAINER(notebook), GUI_PAD);

    contentvbox = gtk_dialog_get_content_area(GTK_DIALOG(object));
    gtk_box_pack_start(GTK_BOX(contentvbox), notebook, TRUE, TRUE, 0);

    if (priv->show_details) {
        TrgPrefs *prefs = trg_client_get_prefs(priv->client);
        if ((width =
             trg_prefs_get_int(prefs, "dialog-width",
                               TRG_PREFS_GLOBAL)) <= 0
            || (height =
                trg_prefs_get_int(prefs, "dialog-height",
                                  TRG_PREFS_GLOBAL)) <= 0) {
            width = 700;
            height = 600;
        }
    } else {
        width = height = 500;
    }

    gtk_window_set_default_size(window, width, height);

    return object;
}

static void trg_torrent_props_dialog_class_init(TrgTorrentPropsDialogClass
                                                * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructor = trg_torrent_props_dialog_constructor;
    object_class->set_property = trg_torrent_props_dialog_set_property;
    object_class->get_property = trg_torrent_props_dialog_get_property;

    g_type_class_add_private(klass, sizeof(TrgTorrentPropsDialogPrivate));

    g_object_class_install_property(object_class, PROP_TREEVIEW,
                                    g_param_spec_object
                                    ("torrent-tree-view",
                                     "TrgTorrentTreeView",
                                     "TrgTorrentTreeView",
                                     TRG_TYPE_TORRENT_TREE_VIEW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class, PROP_TORRENT_MODEL,
                                    g_param_spec_object("torrent-model",
                                                        "TrgTorrentModel",
                                                        "TrgTorrentModel",
                                                        TRG_TYPE_TORRENT_MODEL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class, PROP_PARENT_WINDOW,
                                    g_param_spec_object("parent-window",
                                                        "Parent window",
                                                        "Parent window",
                                                        TRG_TYPE_MAIN_WINDOW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class, PROP_CLIENT,
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
}

static void trg_torrent_props_dialog_init(TrgTorrentPropsDialog *
                                          self G_GNUC_UNUSED)
{
}

TrgTorrentPropsDialog *trg_torrent_props_dialog_new(GtkWindow * window,
                                                    TrgTorrentTreeView *
                                                    treeview,
                                                    TrgTorrentModel *
                                                    torrentModel,
                                                    TrgClient * client)
{
    return g_object_new(TRG_TYPE_TORRENT_PROPS_DIALOG, "torrent-tree-view",
                        treeview, "torrent-model", torrentModel,
                        "parent-window", window, "trg-client", client,
                        NULL);
}
