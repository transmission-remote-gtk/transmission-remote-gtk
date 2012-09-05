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

#ifndef TRG_MENU_BAR_H_
#define TRG_MENU_BAR_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-prefs.h"
#include "trg-torrent-tree-view.h"
#include "trg-main-window.h"

G_BEGIN_DECLS
#define TRG_TYPE_MENU_BAR trg_menu_bar_get_type()
#define TRG_MENU_BAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_MENU_BAR, TrgMenuBar))
#define TRG_MENU_BAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_MENU_BAR, TrgMenuBarClass))
#define TRG_IS_MENU_BAR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_MENU_BAR))
#define TRG_IS_MENU_BAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_MENU_BAR))
#define TRG_MENU_BAR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_MENU_BAR, TrgMenuBarClass))
    typedef struct {
    GtkMenuBar parent;
} TrgMenuBar;

typedef struct {
    GtkMenuBarClass parent_class;
} TrgMenuBarClass;

GType trg_menu_bar_get_type(void);

TrgMenuBar *trg_menu_bar_new(TrgMainWindow * win, TrgPrefs * prefs,
                             TrgTorrentTreeView * ttv,
                             GtkAccelGroup * accel_group);
GtkWidget *trg_menu_bar_item_new(GtkMenuShell * shell, const gchar * text,
                                 const gchar * stock_id,
                                 gboolean sensitive);

G_END_DECLS
    void trg_menu_bar_torrent_actions_sensitive(TrgMenuBar * mb,
                                                gboolean sensitive);
void trg_menu_bar_connected_change(TrgMenuBar * mb, gboolean connected);
void trg_menu_bar_set_supports_queues(TrgMenuBar * mb,
                                      gboolean supportsQueues);
GtkWidget *trg_menu_bar_file_connect_menu_new(TrgMainWindow * win,
                                              TrgPrefs * p);

#endif                          /* TRG_MENU_BAR_H_ */
