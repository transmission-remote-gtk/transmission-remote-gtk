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
#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

#include <glib-object.h>

#include "trg-client.h"
#include "trg-torrent-model.h"

#define TRG_TYPE_GENERAL_PANEL trg_general_panel_get_type()
G_DECLARE_FINAL_TYPE(TrgGeneralPanel, trg_general_panel, TRG, GENERAL_PANEL, GtkGrid)

TrgGeneralPanel *trg_general_panel_new(GtkTreeModel *model, TrgClient *tc);
void trg_general_panel_update(TrgGeneralPanel *panel, JsonObject *t, GtkTreeIter *iter);
void trg_general_panel_clear(TrgGeneralPanel *panel);
