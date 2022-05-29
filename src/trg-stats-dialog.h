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

#ifndef TRG_STATS_DIALOG_H_
#define TRG_STATS_DIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-client.h"
#include "trg-main-window.h"
#include "trg-tree-view.h"

G_BEGIN_DECLS
#define TRG_TYPE_STATS_DIALOG trg_stats_dialog_get_type()
#define TRG_STATS_DIALOG(obj)                                                                      \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_STATS_DIALOG, TrgStatsDialog))
#define TRG_STATS_DIALOG_CLASS(klass)                                                              \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_STATS_DIALOG, TrgStatsDialogClass))
#define TRG_IS_STATS_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_STATS_DIALOG))
#define TRG_IS_STATS_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_STATS_DIALOG))
#define TRG_STATS_DIALOG_GET_CLASS(obj)                                                            \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_STATS_DIALOG, TrgStatsDialogClass))
typedef struct {
    GtkDialog parent;
} TrgStatsDialog;

typedef struct {
    GtkDialogClass parent_class;
} TrgStatsDialogClass;

GType trg_stats_dialog_get_type(void);

TrgStatsDialog *trg_stats_dialog_get_instance(TrgMainWindow *parent, TrgClient *client);

G_END_DECLS
#endif /* TRG_STATS_DIALOG_H_ */
