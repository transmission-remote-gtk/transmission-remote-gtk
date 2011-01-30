/*
 * transmission-remote-gtk - Transmission RPC client for GTK
 * Copyright (C) 2010  Alan Fitton

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
#include "trg-trackers-tree-view.h"

#include "trg-tree-view.h"

G_DEFINE_TYPE(TrgTrackersTreeView, trg_trackers_tree_view,
	      TRG_TYPE_TREE_VIEW)


static void
trg_trackers_tree_view_class_init(TrgTrackersTreeViewClass * klass)
{
}

static void trg_trackers_tree_view_init(TrgTrackersTreeView * self)
{
    trg_tree_view_add_pixbuf_text_column(TRG_TREE_VIEW(self),
					 TRACKERCOL_ICON,
					 TRACKERCOL_TIER, "Tier", -1);
    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Announce URL",
			     TRACKERCOL_ANNOUNCE);
    trg_tree_view_add_column(TRG_TREE_VIEW(self), "Scrape URL",
			     TRACKERCOL_SCRAPE);
}

TrgTrackersTreeView *trg_trackers_tree_view_new(TrgTrackersModel * model)
{
    GObject *obj = g_object_new(TRG_TYPE_TRACKERS_TREE_VIEW, NULL);
    gtk_tree_view_set_model(GTK_TREE_VIEW(obj), GTK_TREE_MODEL(model));

    return TRG_TRACKERS_TREE_VIEW(obj);
}
