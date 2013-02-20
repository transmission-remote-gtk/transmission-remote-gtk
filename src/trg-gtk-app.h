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

#include <gtk/gtk.h>

#ifndef TRG_GTKAPP_
#define TRG_GTKAPP_
#if GTK_CHECK_VERSION( 3, 0, 0 )

#include <glib-object.h>

#include "trg-client.h"

G_BEGIN_DECLS
#define TRG_TYPE_GTK_APP trg_gtk_app_get_type()
#define TRG_GTK_APP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_GTK_APP, TrgGtkApp))
#define TRG_GTK_APP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_GTK_APP, TrgGtkAppClass))
#define TRG_IS_GTK_APP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_GTK_APP))
#define TRG_IS_GTK_APP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_GTK_APP))
#define TRG_GTK_APP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_GTK_APP, TrgGtkAppClass))
    typedef struct {
    GtkApplication parent;
} TrgGtkApp;

typedef struct {
    GtkApplicationClass parent_class;
} TrgGtkAppClass;

GType trg_gtk_app_get_type(void);

TrgGtkApp *trg_gtk_app_new(TrgClient * client);

#endif
#endif
