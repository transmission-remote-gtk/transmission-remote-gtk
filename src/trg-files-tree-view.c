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

#include <gtk/gtk.h>

#include "trg-client.h"
#include "trg-tree-view.h"
#include "trg-files-tree-view.h"
#include "trg-files-model.h"
#include "trg-cell-renderer-wanted.h"
#include "trg-cell-renderer-priority.h"
#include "trg-main-window.h"
#include "requests.h"
#include "json.h"
#include "dispatch.h"
#include "protocol-constants.h"

G_DEFINE_TYPE(TrgFilesTreeView, trg_files_tree_view, TRG_TYPE_TREE_VIEW)
#define TRG_FILES_TREE_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_FILES_TREE_VIEW, TrgFilesTreeViewPrivate))
typedef struct _TrgFilesTreeViewPrivate TrgFilesTreeViewPrivate;

struct _TrgFilesTreeViewPrivate {
    trg_client *client;
    TrgMainWindow *win;
};

static void trg_files_tree_view_class_init(TrgFilesTreeViewClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgFilesTreeViewPrivate));
}

static void set_unwanted_foreachfunc(GtkTreeModel * model,
				     GtkTreePath * path G_GNUC_UNUSED,
				     GtkTreeIter * iter,
				     gpointer data G_GNUC_UNUSED)
{
    gtk_list_store_set(GTK_LIST_STORE(model), iter, FILESCOL_WANTED,
		       FALSE, FILESCOL_ICON, GTK_STOCK_CANCEL, -1);
}

static void set_wanted_foreachfunc(GtkTreeModel * model,
				   GtkTreePath * path G_GNUC_UNUSED,
				   GtkTreeIter * iter,
				   gpointer data G_GNUC_UNUSED)
{
    gtk_list_store_set(GTK_LIST_STORE(model), iter, FILESCOL_WANTED,
		       TRUE, FILESCOL_ICON, GTK_STOCK_FILE, -1);
}

static void set_low_foreachfunc(GtkTreeModel * model,
				GtkTreePath * path G_GNUC_UNUSED,
				GtkTreeIter * iter,
				gpointer data G_GNUC_UNUSED)
{
    gtk_list_store_set(GTK_LIST_STORE(model), iter, FILESCOL_PRIORITY,
		       T_PRIORITY_LOW, -1);
}

static void set_normal_foreachfunc(GtkTreeModel * model,
				   GtkTreePath * path G_GNUC_UNUSED,
				   GtkTreeIter * iter,
				   gpointer data G_GNUC_UNUSED)
{
    gtk_list_store_set(GTK_LIST_STORE(model), iter, FILESCOL_PRIORITY,
		       T_PRIORITY_NORMAL, -1);
}

static void set_high_foreachfunc(GtkTreeModel * model,
				 GtkTreePath * path G_GNUC_UNUSED,
				 GtkTreeIter * iter,
				 gpointer data G_GNUC_UNUSED)
{
    gtk_list_store_set(GTK_LIST_STORE(model), iter, FILESCOL_PRIORITY,
		       T_PRIORITY_HIGH, -1);
}

static void add_file_id_to_array(JsonObject * args, gchar * key,
				 gint index)
{
    JsonArray *array = json_object_get_array_member(args, key);
    json_array_add_int_element(array, index);
}

static void send_updated_file_prefs_foreachfunc(GtkTreeModel * model,
						GtkTreePath *
						path G_GNUC_UNUSED,
						GtkTreeIter * iter,
						gpointer data)
{
    JsonObject *args = (JsonObject *) data;
    gint64 priority, id;
    gboolean wanted;

    gtk_tree_model_get(model, iter, FILESCOL_WANTED, &wanted,
		       FILESCOL_PRIORITY, &priority, FILESCOL_ID, &id, -1);

    if (wanted == FALSE)
	add_file_id_to_array(args, FIELD_FILES_UNWANTED, id);
    else
	add_file_id_to_array(args, FIELD_FILES_WANTED, id);

    if (priority == T_PRIORITY_LOW)
	add_file_id_to_array(args, FIELD_FILES_PRIORITY_LOW, id);
    else if (priority == T_PRIORITY_HIGH)
	add_file_id_to_array(args, FIELD_FILES_PRIORITY_HIGH, id);
    else
	add_file_id_to_array(args, FIELD_FILES_PRIORITY_NORMAL, id);
}

static void remove_array_if_empty(JsonObject * args, gchar * key)
{
    JsonArray *array = json_object_get_array_member(args, key);
    if (json_array_get_length(array) == 0)
	json_object_remove_member(args, key);
}

static void send_updated_file_prefs(TrgFilesTreeView * tv)
{
    TrgFilesTreeViewPrivate *priv = TRG_FILES_TREE_VIEW_GET_PRIVATE(tv);
    JsonNode *req;
    JsonObject *args;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    gint64 targetId;
    JsonArray *targetIdArray;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
    targetId = trg_files_model_get_torrent_id(TRG_FILES_MODEL(model));
    targetIdArray = json_array_new();
    json_array_add_int_element(targetIdArray, targetId);

    req = torrent_set(targetIdArray);
    args = node_get_arguments(req);

    json_object_set_array_member(args, FIELD_FILES_WANTED,
				 json_array_new());
    json_object_set_array_member(args, FIELD_FILES_UNWANTED,
				 json_array_new());
    json_object_set_array_member(args, FIELD_FILES_PRIORITY_HIGH,
				 json_array_new());
    json_object_set_array_member(args, FIELD_FILES_PRIORITY_NORMAL,
				 json_array_new());
    json_object_set_array_member(args, FIELD_FILES_PRIORITY_LOW,
				 json_array_new());

    gtk_tree_selection_selected_foreach(selection,
					send_updated_file_prefs_foreachfunc,
					args);

    remove_array_if_empty(args, FIELD_FILES_WANTED);
    remove_array_if_empty(args, FIELD_FILES_UNWANTED);
    remove_array_if_empty(args, FIELD_FILES_PRIORITY_HIGH);
    remove_array_if_empty(args, FIELD_FILES_PRIORITY_NORMAL);
    remove_array_if_empty(args, FIELD_FILES_PRIORITY_LOW);

    trg_files_model_set_update_barrier(TRG_FILES_MODEL(model),
				       priv->client->updateSerial + 1);
    dispatch_async(priv->client, req, on_generic_interactive_action,
		   priv->win);
}

static void set_low(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgFilesTreeView *tv = TRG_FILES_TREE_VIEW(data);
    GtkTreeSelection *selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    gtk_tree_selection_selected_foreach(selection, set_low_foreachfunc,
					NULL);
    send_updated_file_prefs(tv);
}

static void set_normal(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgFilesTreeView *tv = TRG_FILES_TREE_VIEW(data);
    GtkTreeSelection *selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    gtk_tree_selection_selected_foreach(selection,
					set_normal_foreachfunc, NULL);
    send_updated_file_prefs(tv);
}

static void set_high(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgFilesTreeView *tv = TRG_FILES_TREE_VIEW(data);
    GtkTreeSelection *selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    gtk_tree_selection_selected_foreach(selection,
					set_high_foreachfunc, NULL);
    send_updated_file_prefs(tv);
}

static void set_unwanted(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgFilesTreeView *tv = TRG_FILES_TREE_VIEW(data);
    GtkTreeSelection *selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    gtk_tree_selection_selected_foreach(selection,
					set_unwanted_foreachfunc, NULL);
    send_updated_file_prefs(tv);
}

static void set_wanted(GtkWidget * w G_GNUC_UNUSED, gpointer data)
{
    TrgFilesTreeView *tv = TRG_FILES_TREE_VIEW(data);
    GtkTreeSelection *selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    gtk_tree_selection_selected_foreach(selection,
					set_wanted_foreachfunc, NULL);
    send_updated_file_prefs(tv);
}

static void
view_popup_menu(GtkWidget * treeview, GdkEventButton * event,
		gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem = gtk_menu_item_new_with_label("High Priority");
    g_signal_connect(menuitem, "activate", G_CALLBACK(set_high), treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label("Normal Priority");
    g_signal_connect(menuitem, "activate", G_CALLBACK(set_normal),
		     treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label("Low Priority");
    g_signal_connect(menuitem, "activate", G_CALLBACK(set_low), treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
			  gtk_separator_menu_item_new());

    menuitem = gtk_image_menu_item_new_with_label(GTK_STOCK_FILE);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(menuitem), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
					      (menuitem), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "Download");
    g_signal_connect(menuitem, "activate", G_CALLBACK(set_wanted),
		     treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_image_menu_item_new_with_label(GTK_STOCK_STOP);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(menuitem), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
					      (menuitem), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "Skip");
    g_signal_connect(menuitem, "activate", G_CALLBACK(set_unwanted),
		     treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		   (event != NULL) ? event->button : 0,
		   gdk_event_get_time((GdkEvent *) event));
}

static gboolean
view_onButtonPressed(GtkWidget * treeview, GdkEventButton * event,
		     gpointer userdata)
{
    GtkTreeSelection *selection;
    GtkTreePath *path;

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
					  (gint) event->x,
					  (gint) event->y, &path,
					  NULL, NULL, NULL)) {
	    if (!gtk_tree_selection_path_is_selected(selection, path)) {
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(selection, path);
	    }
	    gtk_tree_path_free(path);

	    view_popup_menu(treeview, event, userdata);
	    return TRUE;
	}
    }

    return FALSE;
}

static gboolean view_onPopupMenu(GtkWidget * treeview, gpointer userdata)
{
    view_popup_menu(treeview, NULL, userdata);
    return TRUE;
}

static void trg_files_tree_view_add_priority_column(TrgTreeView * tv,
						    char *title, int index,
						    int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = trg_cell_renderer_priority_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "priority-value",
						      index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

static void trg_files_tree_view_add_wanted_column(TrgTreeView * tv,
						  char *title, int index,
						  int width)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = trg_cell_renderer_wanted_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "wanted-value",
						      index, NULL);

    trg_tree_view_std_column_setup(column, index, width);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
}

static void trg_files_tree_view_init(TrgFilesTreeView * self)
{
    trg_tree_view_add_pixbuf_text_column(TRG_TREE_VIEW(self),
					 FILESCOL_ICON, FILESCOL_NAME,
					 "Name", -1);
    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Size", FILESCOL_SIZE);
    trg_tree_view_add_prog_column(TRG_TREE_VIEW(self), "Progress",
				  FILESCOL_PROGRESS, -1);
    trg_files_tree_view_add_wanted_column(TRG_TREE_VIEW(self),
					  "Wanted", FILESCOL_WANTED, -1);
    trg_files_tree_view_add_priority_column(TRG_TREE_VIEW(self),
					    "Priority",
					    FILESCOL_PRIORITY, -1);

    g_signal_connect(self, "button-press-event",
		     G_CALLBACK(view_onButtonPressed), NULL);
    g_signal_connect(self, "popup-menu", G_CALLBACK(view_onPopupMenu),
		     NULL);
}

TrgFilesTreeView *trg_files_tree_view_new(TrgFilesModel * model,
					  TrgMainWindow * win,
					  trg_client * client)
{
    GObject *obj = g_object_new(TRG_TYPE_FILES_TREE_VIEW, NULL);
    TrgFilesTreeViewPrivate *priv = TRG_FILES_TREE_VIEW_GET_PRIVATE(obj);

    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));
    priv->client = client;
    priv->win = win;

    return TRG_FILES_TREE_VIEW(obj);
}
