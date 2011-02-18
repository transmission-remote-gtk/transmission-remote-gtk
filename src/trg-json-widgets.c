/*
 * transmission-remote-gtk - Transmission RPC client for GTK
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

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-json-widgets.h"

void toggle_active_arg_is_sensitive(GtkToggleButton * b, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_toggle_button_get_active(b));
}

void gtk_spin_button_json_int_out(GtkSpinButton * spin, JsonObject * out)
{
    gchar *key = g_object_get_data(G_OBJECT(spin), JSON_OBJECT_KEY);
    json_object_set_int_member(out, key, gtk_spin_button_get_value(spin));
}

void gtk_combo_box_json_string_output(GtkComboBox * c, JsonObject * out)
{
    gchar *key = g_object_get_data(G_OBJECT(c), JSON_OBJECT_KEY);
    gchar *value = g_object_get_data(G_OBJECT(c), JSON_OBJECT_VALUE);
    json_object_set_string_member(out, key, value);
}

void gtk_spin_button_json_double_out(GtkSpinButton * spin,
                                     JsonObject * out)
{
    gchar *key = g_object_get_data(G_OBJECT(spin), JSON_OBJECT_KEY);
    json_object_set_double_member(out, key,
                                  gtk_spin_button_get_value(spin));
}

void gtk_entry_json_output(GtkEntry * e, JsonObject * out)
{
    gchar *key = g_object_get_data(G_OBJECT(e), JSON_OBJECT_KEY);
    json_object_set_string_member(out, key, gtk_entry_get_text(e));
}

void widget_set_json_key(GtkWidget * w, gchar * key)
{
    g_object_set_data_full(G_OBJECT(w), JSON_OBJECT_KEY,
                           g_strdup(key), g_free);
}

gboolean gtk_toggle_button_json_out(GtkToggleButton * button,
                                    JsonObject * out)
{
    gboolean active = gtk_toggle_button_get_active(button);
    gchar *key = g_object_get_data(G_OBJECT(button), JSON_OBJECT_KEY);
    json_object_set_boolean_member(out, key, active);
    return active;
}
