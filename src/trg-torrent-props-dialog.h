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

#ifndef TRG_TORRENT_PROPS_DIALOG_H_
#define TRG_TORRENT_PROPS_DIALOG_H_

#include <glib-object.h>

#include "trg-torrent-tree-view.h"
#include "trg-client.h"

G_BEGIN_DECLS
#define TRG_TYPE_TORRENT_PROPS_DIALOG trg_torrent_props_dialog_get_type()
#define TRG_TORRENT_PROPS_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_TORRENT_PROPS_DIALOG, TrgTorrentPropsDialog))
#define TRG_TORRENT_PROPS_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_TORRENT_PROPS_DIALOG, TrgTorrentPropsDialogClass))
#define TRG_IS_TORRENT_PROPS_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_TORRENT_PROPS_DIALOG))
#define TRG_IS_TORRENT_PROPS_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_TORRENT_PROPS_DIALOG))
#define TRG_TORRENT_PROPS_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_TORRENT_PROPS_DIALOG, TrgTorrentPropsDialogClass))
    typedef struct {
    GtkDialog parent;
} TrgTorrentPropsDialog;

typedef struct {
    GtkDialogClass parent_class;
} TrgTorrentPropsDialogClass;

GType trg_torrent_props_dialog_get_type(void);

TrgTorrentPropsDialog *trg_torrent_props_dialog_new(GtkWindow * window,
                                                    TrgTorrentTreeView *
                                                    treeview,
                                                    TrgTorrentModel *
                                                    torrentModel,
                                                    TrgClient * client);

G_END_DECLS
#endif                          /* TRG_TORRENT_PROPS_DIALOG_H_ */
