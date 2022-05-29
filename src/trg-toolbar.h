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

#ifndef TRG_TOOLBAR_H_
#define TRG_TOOLBAR_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-main-window.h"
#include "trg-prefs.h"

G_BEGIN_DECLS
#define TRG_TYPE_TOOLBAR trg_toolbar_get_type()
#define TRG_TOOLBAR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_TOOLBAR, TrgToolbar))
#define TRG_TOOLBAR_CLASS(klass)                                                                   \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_TOOLBAR, TrgToolbarClass))
#define TRG_IS_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_TOOLBAR))
#define TRG_IS_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_TOOLBAR))
#define TRG_TOOLBAR_GET_CLASS(obj)                                                                 \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_TOOLBAR, TrgToolbarClass))
typedef struct {
    GtkToolbar parent;
} TrgToolbar;

typedef struct {
    GtkToolbarClass parent_class;
} TrgToolbarClass;

GType trg_toolbar_get_type(void);

TrgToolbar *trg_toolbar_new(TrgMainWindow *win, TrgPrefs *prefs);

G_END_DECLS
void trg_toolbar_torrent_actions_sensitive(TrgToolbar *mb, gboolean sensitive);
void trg_toolbar_connected_change(TrgToolbar *tb, gboolean connected);

#endif /* TRG_TOOLBAR_H_ */
