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

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "protocol-constants.h"
#include "trg-files-model-common.h"
#include "trg-files-tree-view-common.h"

static void expand_all_cb(GtkWidget * w, gpointer data)
{
    gtk_tree_view_expand_all(GTK_TREE_VIEW(data));
}

static void collapse_all_cb(GtkWidget * w, gpointer data)
{
    gtk_tree_view_collapse_all(GTK_TREE_VIEW(data));
}

static unsigned get_selected_rows_count(GtkTreeView * tv)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tv);
    return gtk_tree_selection_count_selected_rows(selection);
}

static void
view_popup_menu(GtkWidget * treeview, GdkEventButton * event,
                GCallback rename_cb, GCallback low_cb,
                GCallback normal_cb, GCallback high_cb,
                GCallback wanted_cb, GCallback unwanted_cb,
                gpointer data G_GNUC_UNUSED)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    if (rename_cb) {
        menuitem = gtk_menu_item_new_with_label(_("Rename..."));
        if (get_selected_rows_count(GTK_TREE_VIEW(treeview)) == 1) {
            g_signal_connect(menuitem, "activate", rename_cb, treeview);
        } else {
            /* Rename on multiple files currently not supported */
            gtk_widget_set_sensitive(menuitem, FALSE);
        }
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                              gtk_separator_menu_item_new());
    }

    menuitem = gtk_menu_item_new_with_label(_("High Priority"));
    g_signal_connect(menuitem, "activate", high_cb, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Normal Priority"));
    g_signal_connect(menuitem, "activate", normal_cb, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Low Priority"));
    g_signal_connect(menuitem, "activate", low_cb, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());

    menuitem = gtk_menu_item_new_with_label(_("Download"));
    g_signal_connect(menuitem, "activate", wanted_cb, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Skip"));
    g_signal_connect(menuitem, "activate", unwanted_cb, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());

    menuitem = gtk_menu_item_new_with_label(_("Expand All"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(expand_all_cb),
                     treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Collapse All"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(collapse_all_cb),
                     treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent *) event));
}

gboolean
trg_files_tree_view_viewOnPopupMenu(GtkWidget * treeview,
                                    GCallback rename_cb,
                                    GCallback low_cb,
                                    GCallback normal_cb,
                                    GCallback high_cb,
                                    GCallback wanted_cb,
                                    GCallback unwanted_cb,
                                    gpointer userdata)
{
    view_popup_menu(treeview, NULL, rename_cb, low_cb, normal_cb, high_cb, wanted_cb,
                    unwanted_cb, userdata);
    return TRUE;
}

static gboolean
onViewPathToggled(GtkTreeView * view,
                  GtkTreeViewColumn * col,
                  GtkTreePath * path,
                  gint pri_id, gint enabled_id, gpointer data)
{
    int cid;
    gboolean handled = FALSE;

    if (!col || !path)
        return FALSE;

    cid = gtk_tree_view_column_get_sort_column_id(col);
    if ((cid == pri_id) || (cid == enabled_id)) {
        GtkTreeIter iter;
        GtkTreeModel *model = gtk_tree_view_get_model(view);

        gtk_tree_model_get_iter(model, &iter, path);

        if (cid == pri_id) {
            int priority;
            gtk_tree_model_get(model, &iter, pri_id, &priority, -1);
            switch (priority) {
            case TR_PRI_NORMAL:
                priority = TR_PRI_HIGH;
                break;
            case TR_PRI_HIGH:
                priority = TR_PRI_LOW;
                break;
            default:
                priority = TR_PRI_NORMAL;
                break;
            }
            trg_files_tree_model_set_subtree(model, path, &iter, pri_id,
                                             priority);
        } else if (cid == enabled_id) {
            int enabled;
            gtk_tree_model_get(model, &iter, enabled_id, &enabled, -1);
            enabled = !enabled;

            trg_files_tree_model_set_subtree(model, path, &iter,
                                             enabled_id, enabled);
        }

        handled = TRUE;
    }

    return handled;
}

static gboolean
getAndSelectEventPath(GtkTreeView * treeview,
                      GdkEventButton * event,
                      GtkTreeViewColumn ** col, GtkTreePath ** path)
{
    GtkTreeSelection *sel;

    if (gtk_tree_view_get_path_at_pos
        (treeview, event->x, event->y, path, col, NULL, NULL)) {
        sel = gtk_tree_view_get_selection(treeview);
        if (!gtk_tree_selection_path_is_selected(sel, *path)) {
            gtk_tree_selection_unselect_all(sel);
            gtk_tree_selection_select_path(sel, *path);
        }
        return TRUE;
    }

    return FALSE;
}

gboolean
trg_files_tree_view_onViewButtonPressed(GtkWidget * w,
                                        GdkEventButton * event,
                                        gint pri_id,
                                        gint enabled_id,
                                        GCallback rename_cb,
                                        GCallback low_cb,
                                        GCallback normal_cb,
                                        GCallback high_cb,
                                        GCallback wanted_cb,
                                        GCallback unwanted_cb,
                                        gpointer gdata)
{
    GtkTreeViewColumn *col = NULL;
    GtkTreePath *path = NULL;
    GtkTreeSelection *selection;
    gboolean handled = FALSE;
    GtkTreeView *treeview = GTK_TREE_VIEW(w);

    if (event->type == GDK_BUTTON_PRESS && event->button == 1
        && !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
        && getAndSelectEventPath(treeview, event, &col, &path)) {
        handled =
            onViewPathToggled(treeview, col, path, pri_id, enabled_id,
                              NULL);
    } else if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                          (gint) event->x, (gint) event->y,
                                          &path, NULL, NULL, NULL)) {
            if (!gtk_tree_selection_path_is_selected(selection, path)) {
                gtk_tree_selection_unselect_all(selection);
                gtk_tree_selection_select_path(selection, path);
            }

            view_popup_menu(w, event, rename_cb, low_cb, normal_cb, high_cb,
                            wanted_cb, unwanted_cb, gdata);
            handled = TRUE;
        }
    }

    gtk_tree_path_free(path);

    return handled;
}
