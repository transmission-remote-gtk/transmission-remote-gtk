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

#ifndef TRG_RSS_WINDOW_H_
#define TRG_RSS_WINDOW_H_

#if HAVE_RSS

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-main-window.h"

G_BEGIN_DECLS
#define TRG_TYPE_RSS_WINDOW trg_rss_window_get_type()
#define TRG_RSS_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_RSS_WINDOW, TrgRssWindow))
#define TRG_RSS_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_RSS_WINDOW, TrgRssWindowClass))
#define TRG_IS_RSS_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_RSS_WINDOW))
#define TRG_IS_RSS_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_RSS_WINDOW))
#define TRG_RSS_WINDOW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_RSS_WINDOW, TrgRssWindowClass))
    typedef struct {
    GtkWindow parent;
} TrgRssWindow;

typedef struct {
    GtkWindowClass parent_class;
} TrgRssWindowClass;

GType trg_rss_window_get_type(void);

TrgRssWindow *trg_rss_window_get_instance(TrgMainWindow *parent, TrgClient *client);

G_END_DECLS

#endif

#endif                          /* TRG_RSS_WINDOW_H_ */
