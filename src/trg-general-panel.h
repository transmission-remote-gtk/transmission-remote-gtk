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

#ifndef TRG_GENERAL_PANEL_H_
#define TRG_GENERAL_PANEL_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include <glib-object.h>

#include "trg-client.h"
#include "trg-torrent-model.h"

G_BEGIN_DECLS
#define TRG_TYPE_GENERAL_PANEL trg_general_panel_get_type()
#define TRG_GENERAL_PANEL(obj)                                                                     \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_GENERAL_PANEL, TrgGeneralPanel))
#define TRG_GENERAL_PANEL_CLASS(klass)                                                             \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_GENERAL_PANEL, TrgGeneralPanelClass))
#define TRG_IS_GENERAL_PANEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_GENERAL_PANEL))
#define TRG_IS_GENERAL_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_GENERAL_PANEL))
#define TRG_GENERAL_PANEL_GET_CLASS(obj)                                                           \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_GENERAL_PANEL, TrgGeneralPanelClass))
typedef struct {
    GtkGrid parent;
} TrgGeneralPanel;

typedef struct {
    GtkGridClass parent_class;
} TrgGeneralPanelClass;

GType trg_general_panel_get_type(void);

TrgGeneralPanel *trg_general_panel_new(GtkTreeModel *model, TrgClient *tc);

G_END_DECLS
void trg_general_panel_update(TrgGeneralPanel *panel, JsonObject *t, GtkTreeIter *iter);
void trg_general_panel_clear(TrgGeneralPanel *panel);

#endif /* TRG_GENERAL_PANEL_H_ */
