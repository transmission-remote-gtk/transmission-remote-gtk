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

/* Most of the UI code was taken from open-dialog.c and files-list.c
 * in Transmission, adapted to fit in with different torrent file parser
 * and JSON dispatch.
 */

#include "config.h"

#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "hig.h"
#include "json.h"
#include "protocol-constants.h"
#include "requests.h"
#include "torrent.h"
#include "trg-cell-renderer-file-icon.h"
#include "trg-cell-renderer-priority.h"
#include "trg-cell-renderer-size.h"
#include "trg-cell-renderer-wanted.h"
#include "trg-client.h"
#include "trg-destination-combo.h"
#include "trg-file-parser.h"
#include "trg-files-model-common.h"
#include "trg-files-tree-view-common.h"
#include "trg-main-window.h"
#include "trg-prefs.h"
#include "trg-torrent-add-dialog.h"
#include "upload.h"
#include "util.h"

enum {
    PROP_0,
    PROP_FILENAME,
    PROP_PARENT,
    PROP_CLIENT,
    PROP_UPLOAD
};

enum {
    FC_INDEX,
    FC_LABEL,
    FC_SIZE,
    FC_PRIORITY,
    FC_ENABLED,
    N_FILE_COLS
};

struct _TrgTorrentAddDialog {
    GtkDialog parent;

    TrgClient *client;
    TrgMainWindow *parent_win;
    GSList *filenames;
    trg_upload *upload;
    GtkWidget *source_chooser;
    GtkWidget *dest_combo;
    GtkWidget *priority_combo;
    GtkWidget *file_list;
    GtkTreeStore *store;
    GtkWidget *paused_check;
    GtkWidget *delete_check;
    guint n_files;
};

G_DEFINE_TYPE(TrgTorrentAddDialog, trg_torrent_add_dialog, GTK_TYPE_DIALOG)

#define MAGNET_MAX_LINK_WIDTH 75

static void trg_torrent_add_dialog_set_property(GObject *object, guint prop_id, const GValue *value,
                                                GParamSpec *pspec G_GNUC_UNUSED)
{
    TrgTorrentAddDialog *self = TRG_TORRENT_ADD_DIALOG(object);
    switch (prop_id) {
    case PROP_FILENAME:
        self->filenames = g_value_get_pointer(value);
        break;
    case PROP_PARENT:
        self->parent_win = g_value_get_object(value);
        break;
    case PROP_UPLOAD:
        self->upload = g_value_get_pointer(value);
        break;
    case PROP_CLIENT:
        self->client = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void trg_torrent_add_dialog_get_property(GObject *object, guint prop_id, GValue *value,
                                                GParamSpec *pspec G_GNUC_UNUSED)
{
    TrgTorrentAddDialog *self = TRG_TORRENT_ADD_DIALOG(object);
    switch (prop_id) {
    case PROP_FILENAME:
        g_value_set_pointer(value, self->filenames);
        break;
    case PROP_PARENT:
        g_value_set_object(value, self->parent_win);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static gboolean add_file_indexes_foreachfunc(GtkTreeModel *model, GtkTreePath *path G_GNUC_UNUSED,
                                             GtkTreeIter *iter, gpointer data)
{
    trg_upload *upload = (trg_upload *)data;
    gint priority, index, wanted;

    gtk_tree_model_get(model, iter, FC_PRIORITY, &priority, FC_ENABLED, &wanted, FC_INDEX, &index,
                       -1);

    if (gtk_tree_model_iter_has_child(model, iter) || index < 0)
        return FALSE;

    upload->file_wanted[index] = wanted;
    upload->file_priorities[index] = priority;

    return FALSE;
}

static void trg_torrent_add_response_cb(GtkDialog *dlg, gint res_id, gpointer data)
{
    TrgTorrentAddDialog *d = TRG_TORRENT_ADD_DIALOG(dlg);
    guint flags = 0x00;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(d->paused_check)))
        flags |= TORRENT_ADD_FLAG_PAUSED;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(d->delete_check)))
        flags |= TORRENT_ADD_FLAG_DELETE;

    if (res_id == GTK_RESPONSE_ACCEPT) {
        gint priority = gtk_combo_box_get_active(GTK_COMBO_BOX(d->priority_combo)) - 1;
        gchar *dir = trg_destination_combo_get_dir(TRG_DESTINATION_COMBO(d->dest_combo));
        trg_upload *upload;

        if (d->upload) {
            upload = d->upload;
        } else {
            upload = g_new0(trg_upload, 1);
            upload->list = d->filenames;
        }

        upload->main_window = d->parent_win;
        upload->client = d->client;
        upload->dir = dir;
        upload->priority = priority;
        upload->flags = flags;
        upload->extra_args = TRUE;

        upload->n_files = d->n_files;
        upload->file_priorities = g_new0(gint, d->n_files);
        upload->file_wanted = g_new0(gint, d->n_files);

        gtk_tree_model_foreach(GTK_TREE_MODEL(d->store), add_file_indexes_foreachfunc, upload);

        trg_do_upload(upload);

        trg_destination_combo_save_selection(TRG_DESTINATION_COMBO(d->dest_combo));
    } else {
        g_str_slist_free(d->filenames);
    }

    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void set_low(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    trg_files_tree_model_set_priority(GTK_TREE_VIEW(data), FC_PRIORITY, TR_PRI_LOW);
}

static void set_normal(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    trg_files_tree_model_set_priority(GTK_TREE_VIEW(data), FC_PRIORITY, TR_PRI_NORMAL);
}

static void set_high(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    trg_files_tree_model_set_priority(GTK_TREE_VIEW(data), FC_PRIORITY, TR_PRI_HIGH);
}

static void set_unwanted(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    trg_files_model_set_wanted(GTK_TREE_VIEW(data), FC_ENABLED, FALSE);
}

static void set_wanted(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    trg_files_model_set_wanted(GTK_TREE_VIEW(data), FC_ENABLED, TRUE);
}

static gboolean onViewButtonPressed(GtkWidget *w, GdkEventButton *event, gpointer gdata)
{
    return trg_files_tree_view_onViewButtonPressed(
        w, event, FC_PRIORITY, FC_ENABLED, NULL, /* no rename */
        G_CALLBACK(set_low), G_CALLBACK(set_normal), G_CALLBACK(set_high), G_CALLBACK(set_wanted),
        G_CALLBACK(set_unwanted), gdata);
}

static GtkWidget *gtr_file_list_new(GtkTreeStore **store)
{
    int size;
    int width;
    GtkWidget *view;
    GtkWidget *scroll;
    GtkCellRenderer *rend;
    GtkTreeSelection *sel;
    GtkTreeViewColumn *col;
    GtkTreeView *tree_view;
    const char *title;
    PangoLayout *pango_layout;
    PangoContext *pango_context;
    PangoFontDescription *pango_font_description;

    /* create the view */
    view = gtk_tree_view_new();
    tree_view = GTK_TREE_VIEW(view);
    gtk_container_set_border_width(GTK_CONTAINER(view), GUI_PAD_BIG);
    g_signal_connect(view, "button-press-event", G_CALLBACK(onViewButtonPressed), view);

    pango_context = gtk_widget_create_pango_context(view);
    pango_font_description
        = pango_font_description_copy(pango_context_get_font_description(pango_context));
    size = pango_font_description_get_size(pango_font_description);
    pango_font_description_set_size(pango_font_description, size * 0.8);
    g_object_unref(G_OBJECT(pango_context));

    /* set up view */
    sel = gtk_tree_view_get_selection(tree_view);
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
    gtk_tree_view_expand_all(tree_view);
    gtk_tree_view_set_search_column(tree_view, FC_LABEL);

    /* add file column */
    col = GTK_TREE_VIEW_COLUMN(
        g_object_new(GTK_TYPE_TREE_VIEW_COLUMN, "expand", TRUE, "title", _("Name"), NULL));
    gtk_tree_view_column_set_resizable(col, TRUE);
    rend = trg_cell_renderer_file_icon_new();
    gtk_tree_view_column_pack_start(col, rend, FALSE);
    gtk_tree_view_column_set_attributes(col, rend, "file-name", FC_LABEL, "file-id", FC_INDEX,
                                        NULL);

    /* add text renderer */
    rend = gtk_cell_renderer_text_new();
    g_object_set(rend, "ellipsize", PANGO_ELLIPSIZE_END, "font-desc", pango_font_description, NULL);
    gtk_tree_view_column_pack_start(col, rend, TRUE);
    gtk_tree_view_column_set_attributes(col, rend, "text", FC_LABEL, NULL);
    gtk_tree_view_column_set_sort_column_id(col, FC_LABEL);
    gtk_tree_view_append_column(tree_view, col);

    /* add "size" column */

    title = _("Size");
    rend = trg_cell_renderer_size_new();
    g_object_set(rend, "alignment", PANGO_ALIGN_RIGHT, "font-desc", pango_font_description, "xpad",
                 GUI_PAD, "xalign", 1.0f, "yalign", 0.5f, NULL);
    col = gtk_tree_view_column_new_with_attributes(title, rend, NULL);
    gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_sort_column_id(col, FC_SIZE);
    gtk_tree_view_column_set_attributes(col, rend, "size-value", FC_SIZE, NULL);
    gtk_tree_view_append_column(tree_view, col);

    /* add "enabled" column */
    title = _("Download");
    pango_layout = gtk_widget_create_pango_layout(view, title);
    pango_layout_get_pixel_size(pango_layout, &width, NULL);
    width += 30; /* room for the sort indicator */
    g_object_unref(G_OBJECT(pango_layout));
    rend = trg_cell_renderer_wanted_new();
    col = gtk_tree_view_column_new_with_attributes(title, rend, "wanted-value", FC_ENABLED, NULL);
    gtk_tree_view_column_set_fixed_width(col, width);
    gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_sort_column_id(col, FC_ENABLED);
    gtk_tree_view_append_column(tree_view, col);

    /* add priority column */
    title = _("Priority");
    pango_layout = gtk_widget_create_pango_layout(view, title);
    pango_layout_get_pixel_size(pango_layout, &width, NULL);
    width += 30; /* room for the sort indicator */
    g_object_unref(G_OBJECT(pango_layout));
    rend = trg_cell_renderer_priority_new();
    col = gtk_tree_view_column_new_with_attributes(title, rend, "priority-value", FC_PRIORITY,
                                                   NULL);
    gtk_tree_view_column_set_fixed_width(col, width);
    gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_sort_column_id(col, FC_PRIORITY);
    gtk_tree_view_append_column(tree_view, col);

    *store = gtk_tree_store_new(N_FILE_COLS, G_TYPE_INT, /* index */
                                G_TYPE_STRING, /* label */
                                G_TYPE_INT64, /* size */
                                G_TYPE_INT, /* priority */
                                G_TYPE_INT); /* dl enabled */

    gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(*store));
    g_object_unref(G_OBJECT(*store));

    /* create the scrolled window and stick the view in it */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(scroll), view);
    gtk_widget_set_size_request(scroll, -1, 200);

    pango_font_description_free(pango_font_description);
    return scroll;
}

static GtkWidget *gtr_dialog_get_content_area(GtkDialog *dialog)
{
    return gtk_dialog_get_content_area(dialog);
}

static void gtr_dialog_set_content(GtkDialog *dialog, GtkWidget *content)
{
    GtkWidget *vbox = gtr_dialog_get_content_area(dialog);
    gtk_box_pack_start(GTK_BOX(vbox), content, TRUE, TRUE, 0);
    gtk_widget_show_all(content);
}

static GtkWidget *gtr_priority_combo_new(void)
{
    return gtr_combo_box_new_enum(_("Low"), TR_PRI_LOW, _("Normal"), TR_PRI_NORMAL, _("High"),
                                  TR_PRI_HIGH, NULL);
}

static void addTorrentFilters(GtkFileChooser *chooser)
{
    GtkFileFilter *filter;

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Torrent files"));
    gtk_file_filter_add_pattern(filter, "*.torrent");
    gtk_file_chooser_add_filter(chooser, filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All files"));
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(chooser, filter);
}

static void store_add_node(GtkTreeStore *store, GtkTreeIter *parent, trg_files_tree_node *node,
                           guint *n_files)
{
    GtkTreeIter child;
    GList *li;

    if (node->name) {
        gtk_tree_store_append(store, &child, parent);
        gtk_tree_store_set(store, &child, FC_LABEL, node->name, FC_ENABLED, 1, FC_INDEX,
                           node->index, FC_PRIORITY, TR_PRI_NORMAL, FC_SIZE, node->length, -1);

        if (!node->children)
            *n_files = *n_files + 1;
    }

    for (li = node->children; li; li = g_list_next(li))
        store_add_node(store, node->name ? &child : NULL, (trg_files_tree_node *)li->data, n_files);
}

static void torrent_not_found_error(GtkWindow *parent, gchar *file)
{
    GtkWidget *dialog
        = gtk_message_dialog_new(parent, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_OK, _("Unable to open torrent file: %s"), file);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void trg_torrent_add_dialog_set_filenames(TrgTorrentAddDialog *d, GSList *filenames)
{
    GtkButton *chooser = GTK_BUTTON(d->source_chooser);
    gint nfiles = filenames ? g_slist_length(filenames) : 0;

    gtk_tree_store_clear(d->store);

    if (d->upload) {
        trg_upload_free(d->upload);
        d->upload = NULL;
    }

    if (nfiles == 1) {
        gchar *file_name = (gchar *)filenames->data;
        if (is_url(file_name) || is_magnet(file_name)) {
            if (strlen(file_name) > MAGNET_MAX_LINK_WIDTH) {
                gchar *file_name_trunc = g_strndup(file_name, MAGNET_MAX_LINK_WIDTH);
                gchar *file_name_trunc_fmt = g_strdup_printf("%s ...", file_name_trunc);
                gtk_button_set_label(chooser, file_name_trunc_fmt);
                g_free(file_name_trunc);
                g_free(file_name_trunc_fmt);
            } else {
                gtk_button_set_label(chooser, file_name);
            }

            gtk_widget_set_sensitive(d->file_list, FALSE);
            gtk_widget_set_sensitive(d->delete_check, FALSE);
        } else {
            gchar *file_name_base;
            trg_torrent_file *tor_data = NULL;

            file_name_base = g_path_get_basename(file_name);

            if (file_name_base) {
                gtk_button_set_label(chooser, file_name_base);
            } else {
                gtk_button_set_label(chooser, file_name);
            }

            if (g_file_test(file_name, G_FILE_TEST_IS_REGULAR)) {
                g_autoptr(GError) error = NULL;
                tor_data = trg_parse_torrent_file(file_name, &error);
                if (!tor_data) {
                    if (error) {
                        trg_error_dialog(GTK_WINDOW(d->parent_win), error->message);
                        d->filenames = NULL;
                        return;
                    }
                } else {
                    store_add_node(d->store, NULL, tor_data->top_node, &d->n_files);
                    trg_torrent_file_free(tor_data);
                }
            } else {
                torrent_not_found_error(GTK_WINDOW(d->parent_win), file_name);
            }

            gtk_widget_set_sensitive(d->file_list, tor_data != NULL);
        }
    } else {
        gtk_widget_set_sensitive(d->file_list, FALSE);
        if (nfiles < 1) {
            gtk_button_set_label(chooser, _("(None)"));
        } else {
            gtk_button_set_label(chooser, _("(Multiple)"));
        }
    }

    d->filenames = filenames;
}

static void trg_torrent_add_dialog_generic_save_dir(GtkFileChooser *c, TrgPrefs *prefs)
{
    gchar *cwd = gtk_file_chooser_get_current_folder(c);

    if (cwd) {
        trg_prefs_set_string(prefs, TRG_PREFS_KEY_LAST_TORRENT_DIR, cwd, TRG_PREFS_GLOBAL);
        g_free(cwd);
    }
}

static GtkFileChooserNative *trg_torrent_add_dialog_generic(GtkWindow *parent, TrgPrefs *prefs)
{
    GtkFileChooserNative *fc = gtk_file_chooser_native_new(
        _("Add a Torrent"), parent, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Add"), _("_Cancel"));
    gchar *dir = trg_prefs_get_string(prefs, TRG_PREFS_KEY_LAST_TORRENT_DIR, TRG_PREFS_GLOBAL);
    if (dir) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fc), dir);
        g_free(dir);
    }

    addTorrentFilters(GTK_FILE_CHOOSER(fc));
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(fc), TRUE);
    return fc;
}

static void trg_torrent_add_dialog_source_click_cb(GtkWidget *w, gpointer data)
{
    TrgTorrentAddDialog *d = TRG_TORRENT_ADD_DIALOG(data);
    g_autoptr(GtkFileChooserNative) chooser
        = trg_torrent_add_dialog_generic(GTK_WINDOW(data), trg_client_get_prefs(d->client));

    if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
        if (d->filenames)
            g_str_slist_free(d->filenames);

        d->filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(chooser));
        trg_torrent_add_dialog_generic_save_dir(GTK_FILE_CHOOSER(chooser),
                                                trg_client_get_prefs(d->client));
        trg_torrent_add_dialog_set_filenames(TRG_TORRENT_ADD_DIALOG(data), d->filenames);
    }
}

static gboolean apply_all_changed_foreachfunc(GtkTreeModel *model, GtkTreePath *path,
                                              GtkTreeIter *iter, gpointer data)
{
    GtkComboBox *combo = GTK_COMBO_BOX(data);
    GtkTreeModel *combo_model = gtk_combo_box_get_model(combo);
    GtkTreeIter selection_iter;
    if (gtk_combo_box_get_active_iter(combo, &selection_iter)) {
        gint column;
        gint value;
        GValue gvalue = { 0 };
        g_value_init(&gvalue, G_TYPE_INT);
        gtk_tree_model_get(combo_model, &selection_iter, 2, &column, 3, &value, -1);
        g_value_set_int(&gvalue, value);
        gtk_tree_store_set_value(GTK_TREE_STORE(model), iter, column, &gvalue);
    }
    return FALSE;
}

static void trg_torrent_add_dialog_apply_all_changed_cb(GtkWidget *w, gpointer data)
{
    TrgTorrentAddDialog *d = TRG_TORRENT_ADD_DIALOG(data);
    GtkWidget *tv = gtk_bin_get_child(GTK_BIN(d->file_list));
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    gtk_tree_model_foreach(model, apply_all_changed_foreachfunc, w);
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), -1);
}

static GtkWidget *trg_torrent_add_dialog_apply_all_combo_new(TrgTorrentAddDialog *dialog)
{
    GtkListStore *model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT);
    GtkWidget *combo = gtk_combo_box_new();
    GtkTreeIter iter;
    GtkCellRenderer *renderer;

    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, 0, _("High Priority"), 1, FC_PRIORITY, 2, TR_PRI_HIGH, -1);
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, 0, _("Normal Priority"), 1, FC_PRIORITY, 2, TR_PRI_NORMAL, -1);
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, 0, _("Low Priority"), 1, FC_PRIORITY, 2, TR_PRI_LOW, -1);
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, 0, _("Download"), 1, FC_ENABLED, 2, TRUE, -1);
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, 0, _("Skip"), 1, FC_ENABLED, 2, FALSE, -1);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combo), renderer, "text", 0);

    gtk_combo_box_set_model(GTK_COMBO_BOX(combo), GTK_TREE_MODEL(model));
    g_signal_connect(combo, "changed", G_CALLBACK(trg_torrent_add_dialog_apply_all_changed_cb),
                     dialog);

    return combo;
}

static GObject *trg_torrent_add_dialog_constructor(GType type, guint n_construct_properties,
                                                   GObjectConstructParam *construct_params)
{
    GObject *obj = G_OBJECT_CLASS(trg_torrent_add_dialog_parent_class)
                       ->constructor(type, n_construct_properties, construct_params);
    TrgTorrentAddDialog *self = TRG_TORRENT_ADD_DIALOG(obj);
    TrgPrefs *prefs = trg_client_get_prefs(self->client);
    GtkWidget *t, *applyall_combo;
    guint row = 0;

    /* window */
    gtk_window_set_title(GTK_WINDOW(obj), _("Add Torrent"));
    gtk_window_set_transient_for(GTK_WINDOW(obj), GTK_WINDOW(self->parent_win));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(obj), TRUE);

    /* buttons */
    gtk_dialog_add_button(GTK_DIALOG(obj), _("_Cancel"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(obj), _("_Open"), GTK_RESPONSE_ACCEPT);
    gtk_dialog_set_default_response(GTK_DIALOG(obj), GTK_RESPONSE_ACCEPT);
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(obj), GTK_RESPONSE_ACCEPT));

    /* workspace */
    t = hig_workarea_create();
    // gtk_container_set_border_width(GTK_CONTAINER(t), GUI_PAD_BIG);

    self->file_list = gtr_file_list_new(&self->store);
    gtk_widget_set_sensitive(self->file_list, FALSE);

    self->paused_check = gtk_check_button_new_with_mnemonic(_("Start _paused"));
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(self->paused_check),
        trg_prefs_get_bool(prefs, TRG_PREFS_KEY_START_PAUSED, TRG_PREFS_GLOBAL));

    self->delete_check
        = gtk_check_button_new_with_mnemonic(_("Delete local .torrent file after adding"));
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(self->delete_check),
        trg_prefs_get_bool(prefs, TRG_PREFS_KEY_DELETE_LOCAL_TORRENT, TRG_PREFS_GLOBAL));

    self->priority_combo = gtr_priority_combo_new();
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->priority_combo), 1);

    self->source_chooser = gtk_button_new();
    hig_workarea_add_row(t, &row, _("_Torrent file:"), self->source_chooser, NULL);

    if (self->filenames)
        trg_torrent_add_dialog_set_filenames(TRG_TORRENT_ADD_DIALOG(obj), self->filenames);

    g_signal_connect(self->source_chooser, "clicked",
                     G_CALLBACK(trg_torrent_add_dialog_source_click_cb), obj);

    self->dest_combo = trg_destination_combo_new(self->client, TRG_PREFS_KEY_LAST_ADD_DESTINATION);

    hig_workarea_add_row(t, &row, _("_Destination folder:"), self->dest_combo, NULL);

    gtk_widget_set_size_request(self->file_list, 466u, 300u);

    hig_workarea_add_wide_tall_control(t, &row, self->file_list);

    applyall_combo = trg_torrent_add_dialog_apply_all_combo_new(TRG_TORRENT_ADD_DIALOG(obj));

    hig_workarea_add_row(t, &row, _("Apply to all:"), applyall_combo, NULL);

    hig_workarea_add_row(t, &row, _("Torrent _priority:"), self->priority_combo, NULL);

    hig_workarea_add_wide_control(t, &row, self->paused_check);
    hig_workarea_add_wide_control(t, &row, self->delete_check);

    gtr_dialog_set_content(GTK_DIALOG(obj), t);

    g_signal_connect(G_OBJECT(obj), "response", G_CALLBACK(trg_torrent_add_response_cb),
                     self->parent_win);

    return obj;
}

static void trg_torrent_add_dialog_class_init(TrgTorrentAddDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = trg_torrent_add_dialog_set_property;
    object_class->get_property = trg_torrent_add_dialog_get_property;
    object_class->constructor = trg_torrent_add_dialog_constructor;

    g_object_class_install_property(
        object_class, PROP_FILENAME,
        g_param_spec_pointer("filenames", "filenames", "filenames",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                 | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_UPLOAD,
        g_param_spec_pointer("upload", "upload", "upload",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                 | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_CLIENT,
        g_param_spec_pointer("client", "client", "client",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                 | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_PARENT,
        g_param_spec_object("parent", "parent", "parent", TRG_TYPE_MAIN_WINDOW,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

static void trg_torrent_add_dialog_init(TrgTorrentAddDialog *self)
{
}

TrgTorrentAddDialog *trg_torrent_add_dialog_new_from_filenames(TrgMainWindow *parent,
                                                               TrgClient *client, GSList *filenames)
{
    return g_object_new(TRG_TYPE_TORRENT_ADD_DIALOG, "filenames", filenames, "parent", parent,
                        "client", client, NULL);
}

void trg_torrent_add_dialog(TrgMainWindow *win, TrgClient *client)
{
    GtkFileChooserNative *fc;
    GtkWidget *c;
    TrgPrefs *prefs = trg_client_get_prefs(client);

    fc = trg_torrent_add_dialog_generic(GTK_WINDOW(win), prefs);

    c = gtk_check_button_new_with_mnemonic(_("Show _options dialog"));
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(c),
        trg_prefs_get_bool(prefs, TRG_PREFS_KEY_ADD_OPTIONS_DIALOG, TRG_PREFS_GLOBAL));
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(fc), c);

    if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(fc)) == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(fc);
        GtkToggleButton *tb = GTK_TOGGLE_BUTTON(gtk_file_chooser_get_extra_widget(chooser));
        gboolean showOptions = gtk_toggle_button_get_active(tb);
        GSList *l = gtk_file_chooser_get_filenames(chooser);

        trg_torrent_add_dialog_generic_save_dir(GTK_FILE_CHOOSER(fc), prefs);

        if (showOptions) {
            TrgTorrentAddDialog *dialog = trg_torrent_add_dialog_new_from_filenames(win, client, l);
            gtk_widget_show_all(GTK_WIDGET(dialog));
            gtk_window_present(GTK_WINDOW(dialog));
        } else {
            trg_upload *upload = g_new0(trg_upload, 1);

            upload->list = l;
            upload->main_window = win;
            upload->client = client;
            upload->extra_args = FALSE;
            upload->flags = trg_prefs_get_add_flags(prefs);

            trg_do_upload(upload);
        }
    }

    g_object_unref(G_OBJECT(fc));
}
