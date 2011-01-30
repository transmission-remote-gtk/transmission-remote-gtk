/*
 * transmission-remote-gtk - Transmission RPC client for GTK
 * Copyright (C) 2010  Alan Fitton

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

#ifndef TRG_JSON_WIDGETS_H_
#define TRG_JSON_WIDGETS_H_

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#define JSON_OBJECT_KEY         "json-object-key"
#define JSON_OBJECT_VALUE         "json-object-value"

void widget_set_json_key(GtkWidget * w, gchar * key);

void gtk_spin_button_json_int_out(GtkSpinButton * spin, JsonObject * out);
void gtk_spin_button_json_double_out(GtkSpinButton * spin,
				     JsonObject * out);
gboolean gtk_toggle_button_json_out(GtkToggleButton * button,
				    JsonObject * out);
void gtk_entry_json_output(GtkEntry * e, JsonObject * out);
void gtk_combo_box_json_string_output(GtkComboBox * c, JsonObject * out);
void toggle_active_arg_is_sensitive(GtkToggleButton * b, gpointer data);

#endif				/* TRG_JSON_WIDGETS_H_ */
