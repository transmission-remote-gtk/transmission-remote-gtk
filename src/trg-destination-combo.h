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

#ifndef TRG_DESTINATION_COMBO_H_
#define TRG_DESTINATION_COMBO_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "trg-client.h"

G_BEGIN_DECLS

#define TRG_TYPE_DESTINATION_COMBO (trg_destination_combo_get_type())
G_DECLARE_FINAL_TYPE(TrgDestinationCombo, trg_destination_combo, TRG, DESTINATION_COMBO,
                     GtkComboBox)

GtkWidget *trg_destination_combo_new(TrgClient *client, const gchar *lastSelectionKey);
gchar *trg_destination_combo_get_dir(TrgDestinationCombo *combo);
gboolean trg_destination_combo_has_text(TrgDestinationCombo *combo);
void trg_destination_combo_save_selection(TrgDestinationCombo *combo_box);

G_END_DECLS
#endif /* TRG_DESTINATION_COMBO_H_ */
