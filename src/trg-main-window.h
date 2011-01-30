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

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-torrent-model.h"
#include "trg-peers-model.h"
#include "trg-files-model.h"
#include "trg-trackers-model.h"
#include "trg-general-panel.h"
#include "trg-torrent-tree-view.h"
#include "trg-client.h"

G_BEGIN_DECLS
#define TRG_TYPE_MAIN_WINDOW trg_main_window_get_type()
#define TRG_MAIN_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_MAIN_WINDOW, TrgMainWindow))
#define TRG_MAIN_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_MAIN_WINDOW, TrgMainWindowClass))
#define TRG_IS_MAIN_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_MAIN_WINDOW))
#define TRG_IS_MAIN_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_MAIN_WINDOW))
#define TRG_MAIN_WINDOW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_MAIN_WINDOW, TrgMainWindowClass))
    typedef struct {
    GtkWindow parent;
} TrgMainWindow;

typedef struct {
    GtkWindowClass parent_class;
} TrgMainWindowClass;

GType trg_main_window_get_type(void);

TrgMainWindow *trg_main_window_new(trg_client * tc);
gboolean trg_add_from_filename(TrgMainWindow * win, gchar * fileName);
void on_generic_interactive_action(JsonObject * response,
				   int status, gpointer data);
void auto_connect_if_required(TrgMainWindow * win, trg_client * tc);
void on_session_set(JsonObject * response, int status, gpointer data);

G_END_DECLS struct add_torrent_threadfunc_args {
    GSList *list;
    trg_client *client;
    gpointer cb_data;
};

#endif				/* MAIN_WINDOW_H_ */
