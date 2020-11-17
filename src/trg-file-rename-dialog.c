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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "trg-main-window.h"
#include "trg-file-rename-dialog.h"
#include "trg-destination-combo.h"
#include "hig.h"
#include "requests.h"

G_DEFINE_TYPE(TrgFileRenameDialog, trg_file_rename_dialog,
              GTK_TYPE_DIALOG)
#define TRG_FILE_RENAME_DIALOG_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_FILE_RENAME_DIALOG, TrgFileRenameDialogPrivate))
typedef struct _TrgFileRenameDialogPrivate TrgFileRenameDialogPrivate;

struct _TrgFileRenameDialogPrivate {
    TrgClient *client;
    TrgMainWindow *win;
    TrgFilesTreeView *treeview;
    JsonArray *ids;
    gchar *orig_path;
    GtkWidget *rename_button;
    GtkEntryBuffer *name_entry_buf;
};

enum {
    PROP_0,
    PROP_CLIENT,
    PROP_PARENT_WINDOW,
    PROP_TREEVIEW
};

static gchar *
trg_file_rename_dialog_get_file_path(GtkTreeModel* model, const GtkTreeIter *iter)
{
    GtkTreeIter current = *iter;
    GtkTreeIter parent;
    gchar *name;
    gchar *filepath;
    gchar *temp_filepath;

    gtk_tree_model_get(model, &current, FILESCOL_NAME, &filepath, -1);

    while (gtk_tree_model_iter_parent(model, &parent, &current)) {
        gtk_tree_model_get(model, &parent, FILESCOL_NAME, &name, -1);
        temp_filepath = g_strdup_printf("%s/%s", name, filepath);
        g_free(name);
        g_free(filepath);
        filepath = temp_filepath;

        current = parent;
    }

    return filepath;
}

static void
trg_file_rename_response_cb(GtkDialog * dlg, gint res_id, gpointer data)
{
    TrgFileRenameDialogPrivate *priv =
        TRG_FILE_RENAME_DIALOG_GET_PRIVATE(dlg);

    if (res_id == GTK_RESPONSE_ACCEPT) {
        const gchar *name = gtk_entry_buffer_get_text(priv->name_entry_buf);
        JsonNode *request = torrent_rename_path(priv->ids, priv->orig_path,
                                                name);

        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(priv->treeview));
        trg_files_model_set_accept(TRG_FILES_MODEL(model), FALSE);

        dispatch_async(priv->client, request,
                       on_files_update, priv->treeview);
    } else {
        g_free(priv->orig_path);
        json_array_unref(priv->ids);
    }
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void location_changed(GtkComboBox * w, gpointer data)
{
    TrgFileRenameDialogPrivate *priv =
        TRG_FILE_RENAME_DIALOG_GET_PRIVATE(data);
    gtk_widget_set_sensitive(priv->rename_button,
                             gtk_entry_buffer_get_length
                              (priv->name_entry_buf));
}

static GObject *trg_file_rename_dialog_constructor(GType type,
                                                   guint
                                                   n_construct_properties,
                                                   GObjectConstructParam *
                                                   construct_params)
{
    GObject *object = G_OBJECT_CLASS
        (trg_file_rename_dialog_parent_class)->constructor(type,
                                                            n_construct_properties,
                                                            construct_params);
    TrgFileRenameDialogPrivate *priv =
        TRG_FILE_RENAME_DIALOG_GET_PRIVATE(object);

    gint64 target_id;
    guint count;
    gchar *msg;

    GtkWidget *t;
    guint row = 0;

    /* Get file selection info */

    GtkTreeView *tv = GTK_TREE_VIEW(priv->treeview);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tv);
    GtkTreeModel *model;
    gchar *orig_filename;

    GList *list = gtk_tree_selection_get_selected_rows(selection, &model);
    count = g_list_length(list);

    if (list) {
        GtkTreeIter iter;

        gtk_tree_model_get_iter(model, &iter, (GtkTreePath *) list->data);
        gtk_tree_model_get(model, &iter, FILESCOL_NAME, &orig_filename, -1);

        priv->orig_path = trg_file_rename_dialog_get_file_path(model, &iter);
    } else {
        orig_filename = g_strdup("error");
        priv->orig_path = NULL;
    }

    /* Populate dialog widget */

    t = hig_workarea_create();

    priv->name_entry_buf = gtk_entry_buffer_new(orig_filename,
                                                strlen(orig_filename));
    GtkWidget* name_entry = gtk_entry_new_with_buffer(priv->name_entry_buf);
    gtk_entry_set_activates_default(GTK_ENTRY(name_entry), TRUE);

    g_signal_connect(name_entry, "changed",
                     G_CALLBACK(location_changed), object);
    hig_workarea_add_row(t, &row, _("Name:"), name_entry, NULL);

    gtk_window_set_destroy_with_parent(GTK_WINDOW(object), TRUE);

    gtk_dialog_add_button(GTK_DIALOG(object), GTK_STOCK_CANCEL,
                          GTK_RESPONSE_CANCEL);
    priv->rename_button =
        gtk_dialog_add_button(GTK_DIALOG(object), _("Rename"),
                              GTK_RESPONSE_ACCEPT);

    gtk_widget_set_sensitive(priv->rename_button,
                             gtk_entry_buffer_get_length
                              (priv->name_entry_buf));

    gtk_container_set_border_width(GTK_CONTAINER(object), GUI_PAD);

    gtk_dialog_set_default_response(GTK_DIALOG(object),
                                    GTK_RESPONSE_ACCEPT);

    gtk_dialog_set_alternative_button_order(GTK_DIALOG(object),
                                            GTK_RESPONSE_ACCEPT,
                                            GTK_RESPONSE_CANCEL, -1);

    gtk_container_set_border_width(GTK_CONTAINER(t), GUI_PAD);

    gtk_box_pack_start(GTK_BOX
                       (gtk_dialog_get_content_area(GTK_DIALOG(object))),
                       t, TRUE, TRUE, 0);

    target_id = trg_files_model_get_torrent_id(TRG_FILES_MODEL(model));
    priv->ids = json_array_new();
    json_array_add_int_element(priv->ids, target_id);

    if (count == 1) {
        msg = g_strdup_printf(_("Rename %s"), orig_filename);
    } else {
        /* this really shouldn't happen since TrgFilesTreeView checks number of
         * selected files before allowing rename */
        msg = g_strdup_printf(_("INTERNAL ERROR: Rename %d files not supported"),
                              count);
        gtk_widget_set_sensitive(priv->rename_button, FALSE);
        gtk_widget_set_sensitive(name_entry, FALSE);
    }

    gtk_window_set_transient_for(GTK_WINDOW(object),
                                 GTK_WINDOW(priv->win));
    gtk_window_set_title(GTK_WINDOW(object), msg);

    g_signal_connect(G_OBJECT(object),
                     "response",
                     G_CALLBACK(trg_file_rename_response_cb), priv->win);

    g_free(msg);
    g_free(orig_filename);
    g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);

    return object;
}

static void
trg_file_rename_dialog_get_property(GObject * object, guint property_id,
                                     GValue * value, GParamSpec * pspec)
{
    TrgFileRenameDialogPrivate *priv =
        TRG_FILE_RENAME_DIALOG_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    case PROP_PARENT_WINDOW:
        g_value_set_object(value, priv->win);
        break;
    case PROP_TREEVIEW:
        g_value_set_object(value, priv->treeview);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_file_rename_dialog_set_property(GObject * object, guint property_id,
                                     const GValue * value,
                                     GParamSpec * pspec)
{
    TrgFileRenameDialogPrivate *priv =
        TRG_FILE_RENAME_DIALOG_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    case PROP_PARENT_WINDOW:
        priv->win = g_value_get_object(value);
        break;
    case PROP_TREEVIEW:
        priv->treeview = g_value_get_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_file_rename_dialog_class_init(TrgFileRenameDialogClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgFileRenameDialogPrivate));

    object_class->get_property = trg_file_rename_dialog_get_property;
    object_class->set_property = trg_file_rename_dialog_set_property;
    object_class->constructor = trg_file_rename_dialog_constructor;

    g_object_class_install_property(object_class,
                                    PROP_TREEVIEW,
                                    g_param_spec_object
                                    ("files-tree-view",
                                     "TrgFilesTreeView",
                                     "TrgFilesTreeView",
                                     TRG_TYPE_FILES_TREE_VIEW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_PARENT_WINDOW,
                                    g_param_spec_object
                                    ("parent-window", "Parent window",
                                     "Parent window", TRG_TYPE_MAIN_WINDOW,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_CLIENT,
                                    g_param_spec_pointer
                                    ("trg-client", "TClient",
                                     "Client",
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));
}

static void trg_file_rename_dialog_init(TrgFileRenameDialog * self)
{
}

TrgFileRenameDialog *trg_file_rename_dialog_new(TrgMainWindow * win,
                                                  TrgClient * client,
                                                  TrgFilesTreeView * ttv)
{
    return g_object_new(TRG_TYPE_FILE_RENAME_DIALOG,
                        "trg-client", client,
                        "files-tree-view", ttv, "parent-window", win,
                        NULL);
}
