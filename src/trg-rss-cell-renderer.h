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

#ifndef TRG_RSS_CELL_RENDERER_H
#define TRG_RSS_CELL_RENDERER_H

#if HAVE_RSS

#include <gtk/gtk.h>

#define TRG_RSS_CELL_RENDERER_TYPE ( trg_rss_cell_renderer_get_type( ) )

#define TRG_RSS_CELL_RENDERER( o ) \
    ( G_TYPE_CHECK_INSTANCE_CAST( ( o ), \
                                 TRG_RSS_CELL_RENDERER_TYPE, \
                                 TrgRssCellRenderer ) )

typedef struct TrgRssCellRenderer TrgRssCellRenderer;

typedef struct TrgRssCellRendererClass TrgRssCellRendererClass;

struct TrgRssCellRenderer {
    GtkCellRenderer parent;
    struct TrgRssCellRendererPrivate *priv;
};

struct TrgRssCellRendererClass {
    GtkCellRendererClass parent;
};

GType trg_rss_cell_renderer_get_type(void) G_GNUC_CONST;

GtkCellRenderer *trg_rss_cell_renderer_new(void);

#endif

#endif                          /* TRG_RSS_CELL_RENDERER_H */
