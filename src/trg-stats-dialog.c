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
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>

#include "hig.h"
#include "json.h"
#include "requests.h"
#include "trg-client.h"
#include "trg-main-window.h"
#include "trg-stats-dialog.h"
#include "trg-tree-view.h"
#include "util.h"

enum {
    STATCOL_STAT,
    STATCOL_SESSION,
    STATCOL_CUMULAT,
    STATCOL_COLUMNS
};

enum {
    PROP_0,
    PROP_PARENT,
    PROP_CLIENT
};

#define STATS_UPDATE_INTERVAL 5

struct _TrgStatsDialog {
    GtkDialog parent;

    TrgClient *client;
    TrgMainWindow *parent_win;
    guint update_stats_timer_tag;
    GtkWidget *tv;
    GtkListStore *model;
    GtkTreeRowReference *rr_down;
    GtkTreeRowReference *rr_up;
    GtkTreeRowReference *rr_ratio;
    GtkTreeRowReference *rr_files_added;
    GtkTreeRowReference *rr_session_count;
    GtkTreeRowReference *rr_active;
    GtkTreeRowReference *rr_version;
};

G_DEFINE_TYPE(TrgStatsDialog, trg_stats_dialog, GTK_TYPE_DIALOG)

static GObject *instance = NULL;
static gboolean trg_update_stats_timerfunc(gpointer data);
static gboolean on_stats_reply(gpointer data);

static void trg_stats_dialog_get_property(GObject *object, guint property_id, GValue *value,
                                          GParamSpec *pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void trg_stats_dialog_set_property(GObject *object, guint property_id, const GValue *value,
                                          GParamSpec *pspec)
{
    TrgStatsDialog *self = TRG_STATS_DIALOG(object);
    switch (property_id) {
    case PROP_CLIENT:
        self->client = g_value_get_pointer(value);
        break;
    case PROP_PARENT:
        self->parent_win = g_value_get_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_stats_response_cb(GtkDialog *dlg, gint res_id, gpointer data G_GNUC_UNUSED)
{
    TrgStatsDialog *trg_dlg = TRG_STATS_DIALOG(dlg);
    g_clear_handle_id(&trg_dlg->update_stats_timer_tag, g_source_remove);
    gtk_widget_destroy(GTK_WIDGET(dlg));
    instance = NULL;
}

static GtkTreeRowReference *stats_dialog_add_statistic(GtkListStore *model, gchar *name)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeRowReference *rr;

    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, STATCOL_STAT, name, -1);
    path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
    rr = gtk_tree_row_reference_new(GTK_TREE_MODEL(model), path);

    gtk_tree_path_free(path);

    return rr;
}

static void update_statistic(GtkTreeRowReference *rr, gchar *session, gchar *cumulat)
{
    GtkTreePath *path = gtk_tree_row_reference_get_path(rr);
    GtkTreeModel *model = gtk_tree_row_reference_get_model(rr);
    GtkTreeIter iter;

    gtk_tree_model_get_iter(model, &iter, path);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, STATCOL_SESSION, session, STATCOL_CUMULAT,
                       cumulat, -1);

    gtk_tree_path_free(path);
}

static JsonObject *get_session_arg(JsonObject *args)
{
    return json_object_get_object_member(args, "current-stats");
}

static JsonObject *get_cumulat_arg(JsonObject *args)
{
    return json_object_get_object_member(args, "cumulative-stats");
}

static void update_int_stat(JsonObject *args, GtkTreeRowReference *rr, gchar *jsonKey)
{
    gchar session_val[32];
    gchar cumulat_val[32];

    g_snprintf(session_val, sizeof(session_val), "%" G_GINT64_FORMAT,
               json_object_get_int_member(get_session_arg(args), jsonKey));
    g_snprintf(cumulat_val, sizeof(cumulat_val), "%" G_GINT64_FORMAT,
               json_object_get_int_member(get_cumulat_arg(args), jsonKey));

    update_statistic(rr, session_val, cumulat_val);
}

static void update_size_stat(JsonObject *args, GtkTreeRowReference *rr, gchar *jsonKey)
{
    gchar session_val[32];
    gchar cumulat_val[32];

    trg_strlsize(cumulat_val, json_object_get_int_member(get_cumulat_arg(args), jsonKey));
    trg_strlsize(session_val, json_object_get_int_member(get_session_arg(args), jsonKey));

    update_statistic(rr, session_val, cumulat_val);
}

static void update_ratio_stat(JsonObject *args, GtkTreeRowReference *rr, gchar *jsonKeyA,
                              gchar *jsonKeyB)
{
    gchar session_val[32];
    gchar cumulat_val[32];

    trg_strlratio(session_val,
                  json_object_get_double_member(get_session_arg(args), jsonKeyA)
                      / json_object_get_double_member(get_session_arg(args), jsonKeyB));

    trg_strlratio(cumulat_val,
                  json_object_get_double_member(get_cumulat_arg(args), jsonKeyA)
                      / json_object_get_double_member(get_cumulat_arg(args), jsonKeyB));

    update_statistic(rr, session_val, cumulat_val);
}

static void update_time_stat(JsonObject *args, GtkTreeRowReference *rr, gchar *jsonKey)
{
    gchar session_val[32];
    gchar cumulat_val[32];

    tr_strltime_long(session_val, json_object_get_int_member(get_session_arg(args), jsonKey),
                     sizeof(session_val));
    tr_strltime_long(cumulat_val, json_object_get_int_member(get_cumulat_arg(args), jsonKey),
                     sizeof(cumulat_val));

    update_statistic(rr, session_val, cumulat_val);
}

static gboolean on_stats_reply(gpointer data)
{
    trg_response *response = (trg_response *)data;

    if (!TRG_IS_STATS_DIALOG(response->cb_data)) {
        trg_response_free(response);
        return FALSE;
    }

    if (response->status == SOUP_STATUS_OK) {
        TrgStatsDialog *dlg = TRG_STATS_DIALOG(response->cb_data);
        JsonObject *args = get_arguments(response->obj);
        char versionStr[32];

        g_snprintf(versionStr, sizeof(versionStr), "Transmission %s",
                   trg_client_get_version_string(dlg->client));
        update_statistic(dlg->rr_version, versionStr, "");

        update_size_stat(args, dlg->rr_up, "uploadedBytes");
        update_size_stat(args, dlg->rr_down, "downloadedBytes");
        update_ratio_stat(args, dlg->rr_ratio, "uploadedBytes", "downloadedBytes");
        update_int_stat(args, dlg->rr_files_added, "filesAdded");
        update_int_stat(args, dlg->rr_session_count, "sessionCount");
        update_time_stat(args, dlg->rr_active, "secondsActive");

        if (trg_client_is_connected(dlg->client))
            dlg->update_stats_timer_tag = g_timeout_add_seconds(
                STATS_UPDATE_INTERVAL, trg_update_stats_timerfunc, response->cb_data);
    } else {
        trg_client_error_dialog(GTK_WINDOW(data), response);
    }

    trg_response_free(response);
    return FALSE;
}

static gboolean trg_update_stats_timerfunc(gpointer data)
{
    if (TRG_IS_STATS_DIALOG(data)) {
        TrgStatsDialog *dlg = TRG_STATS_DIALOG(data);
        if (trg_client_is_connected(dlg->client))
            dispatch_rpc_async(dlg->client, session_stats(), on_stats_reply, data);
    }

    return FALSE;
}

static void trg_stats_add_column(GtkTreeView *tv, gint index, gchar *title, gint width)
{
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column
        = gtk_tree_view_column_new_with_attributes(title, renderer, "text", index, NULL);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(column, width);

    gtk_tree_view_append_column(tv, column);
}

static GObject *trg_stats_dialog_constructor(GType type, guint n_construct_properties,
                                             GObjectConstructParam *construct_params)
{
    GtkWidget *tv;
    GObject *obj = G_OBJECT_CLASS(trg_stats_dialog_parent_class)
                       ->constructor(type, n_construct_properties, construct_params);
    TrgStatsDialog *dlg = TRG_STATS_DIALOG(obj);

    gtk_window_set_title(GTK_WINDOW(obj), _("Statistics"));
    gtk_window_set_transient_for(GTK_WINDOW(obj), GTK_WINDOW(dlg->parent_win));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(obj), TRUE);
    gtk_dialog_add_button(GTK_DIALOG(obj), _("_Close"), GTK_RESPONSE_CLOSE);

    gtk_container_set_border_width(GTK_CONTAINER(obj), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(obj), GTK_RESPONSE_CLOSE);

    g_signal_connect(G_OBJECT(obj), "response", G_CALLBACK(trg_stats_response_cb), NULL);

    dlg->model = gtk_list_store_new(STATCOL_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    dlg->rr_version = stats_dialog_add_statistic(dlg->model, _("Version"));
    dlg->rr_down = stats_dialog_add_statistic(dlg->model, _("Download Total"));
    dlg->rr_up = stats_dialog_add_statistic(dlg->model, _("Upload Total"));
    dlg->rr_ratio = stats_dialog_add_statistic(dlg->model, _("Ratio"));
    dlg->rr_files_added = stats_dialog_add_statistic(dlg->model, _("Files Added"));
    dlg->rr_session_count = stats_dialog_add_statistic(dlg->model, _("Session Count"));
    dlg->rr_active = stats_dialog_add_statistic(dlg->model, _("Time Active"));

    tv = dlg->tv = trg_tree_view_new();
    gtk_widget_set_sensitive(tv, TRUE);

    trg_stats_add_column(GTK_TREE_VIEW(tv), STATCOL_STAT, _("Statistic"), 200);
    trg_stats_add_column(GTK_TREE_VIEW(tv), STATCOL_SESSION, _("Session"), 200);
    trg_stats_add_column(GTK_TREE_VIEW(tv), STATCOL_CUMULAT, _("Cumulative"), 200);

    gtk_tree_view_set_model(GTK_TREE_VIEW(tv), GTK_TREE_MODEL(dlg->model));

    gtk_container_set_border_width(GTK_CONTAINER(tv), GUI_PAD);
    gtk_box_pack_start(GTK_BOX(gtk_bin_get_child(GTK_BIN(obj))), tv, TRUE, TRUE, 0);

    dispatch_rpc_async(dlg->client, session_stats(), on_stats_reply, obj);

    return obj;
}

static void trg_stats_dialog_class_init(TrgStatsDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_stats_dialog_get_property;
    object_class->set_property = trg_stats_dialog_set_property;
    object_class->constructor = trg_stats_dialog_constructor;

    g_object_class_install_property(
        object_class, PROP_PARENT,
        g_param_spec_object("parent-window", "Parent window", "Parent window", TRG_TYPE_MAIN_WINDOW,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_CLIENT,
        g_param_spec_pointer("trg-client", "TClient", "Client",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                 | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

static void trg_stats_dialog_init(TrgStatsDialog *self)
{
}

TrgStatsDialog *trg_stats_dialog_get_instance(TrgMainWindow *parent, TrgClient *client)
{
    if (instance == NULL) {
        instance = g_object_new(TRG_TYPE_STATS_DIALOG, "trg-client", client, "parent-window",
                                parent, NULL);
    }

    return TRG_STATS_DIALOG(instance);
}
