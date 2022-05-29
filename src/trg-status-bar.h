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

#ifndef TRG_STATUS_BAR_H_
#define TRG_STATUS_BAR_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-torrent-model.h"

G_BEGIN_DECLS
#define TRG_TYPE_STATUS_BAR trg_status_bar_get_type()
#define TRG_STATUS_BAR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_STATUS_BAR, TrgStatusBar))
#define TRG_STATUS_BAR_CLASS(klass)                                                                \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_STATUS_BAR, TrgStatusBarClass))
#define TRG_IS_STATUS_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_STATUS_BAR))
#define TRG_IS_STATUS_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_STATUS_BAR))
#define TRG_STATUS_BAR_GET_CLASS(obj)                                                              \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_STATUS_BAR, TrgStatusBarClass))
typedef struct {
    GtkHBox parent;
} TrgStatusBar;

typedef struct {
    GtkHBoxClass parent_class;
} TrgStatusBarClass;

GType trg_status_bar_get_type(void);

TrgStatusBar *trg_status_bar_new(TrgMainWindow *win, TrgClient *client);

G_END_DECLS
void trg_status_bar_update(TrgStatusBar *sb, trg_torrent_model_update_stats *stats,
                           TrgClient *client);
void trg_status_bar_session_update(TrgStatusBar *sb, JsonObject *session);
void trg_status_bar_connect(TrgStatusBar *sb, JsonObject *session, TrgClient *client);
void trg_status_bar_push_connection_msg(TrgStatusBar *sb, const gchar *msg);
void trg_status_bar_reset(TrgStatusBar *sb);
void trg_status_bar_clear_indicators(TrgStatusBar *sb);
const gchar *trg_status_bar_get_speed_text(TrgStatusBar *s);
void trg_status_bar_update_speed(TrgStatusBar *sb, trg_torrent_model_update_stats *stats,
                                 TrgClient *client);
#endif /* TRG_STATUS_BAR_H_ */
