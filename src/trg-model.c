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

#include <glib.h>
#include <gtk/gtk.h>

struct trg_model_remove_removed_foreachfunc_args {
    gint64 currentSerial;
    gint serial_column;
    GList *toRemove;
};

gboolean
trg_model_remove_removed_foreachfunc(GtkTreeModel * model,
				     GtkTreePath * path G_GNUC_UNUSED,
				     GtkTreeIter * iter, gpointer data)
{
    struct trg_model_remove_removed_foreachfunc_args *args =
	(struct trg_model_remove_removed_foreachfunc_args *) data;
    gint64 rowSerial;
    gtk_tree_model_get(model, iter, args->serial_column, &rowSerial, -1);
    if (rowSerial != args->currentSerial)
	args->toRemove =
	    g_list_append(args->toRemove, gtk_tree_iter_copy(iter));

    return FALSE;
}

void
trg_model_remove_removed(GtkListStore * model, gint serial_column,
			 gint64 currentSerial)
{
    struct trg_model_remove_removed_foreachfunc_args args;
    GList *li;

    args.toRemove = NULL;
    args.currentSerial = currentSerial;
    args.serial_column = serial_column;
    gtk_tree_model_foreach(GTK_TREE_MODEL(model),
			   trg_model_remove_removed_foreachfunc, &args);
    if (args.toRemove != NULL) {
	for (li = g_list_last(args.toRemove); li != NULL;
	     li = g_list_previous(li)) {
	    gtk_list_store_remove(model, (GtkTreeIter *) li->data);
	    gtk_tree_iter_free((GtkTreeIter *) li->data);
	}
	g_list_free(args.toRemove);
    }
}
