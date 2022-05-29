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

#include "config.h"

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "json.h"
#include "trg-json-widgets.h"
#include "util.h"

/* Functions for creating widgets that load/save their state from/to a JSON
 * object. This is used by the torrent properties and remote settings dialogs.
 * The pattern here is farily similar to that used in local configuration,
 * the widget creation functions take a list as an argument, which gets a
 * trg_json_widget_desc appended to it. This contains the key, and the function
 * pointers for load/save.
 */

void trg_json_widgets_save(GList *list, JsonObject *out)
{
    GList *li;
    for (li = list; li; li = g_list_next(li)) {
        trg_json_widget_desc *wd = (trg_json_widget_desc *)li->data;
        wd->saveFunc(wd->widget, out, wd->key);
    }
}

void trg_json_widget_desc_free(trg_json_widget_desc *wd)
{
    g_free(wd->key);
    g_free(wd);
}

void trg_json_widget_desc_list_free(GList *list)
{
    g_list_free_full(list, (GDestroyNotify)trg_json_widget_desc_free);
}

void toggle_active_arg_is_sensitive(GtkToggleButton *b, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data), gtk_toggle_button_get_active(b));
}

GtkWidget *trg_json_widget_check_new(GList **wl, JsonObject *obj, const gchar *key,
                                     const gchar *label, GtkWidget *toggleDep)
{
    GtkWidget *w = gtk_check_button_new_with_mnemonic(label);
    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);

    wd->saveFunc = trg_json_widget_check_save;
    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled", G_CALLBACK(toggle_active_arg_is_sensitive),
                         w);
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), json_object_get_boolean_member(obj, key));

    *wl = g_list_append(*wl, wd);

    return w;
}

GtkWidget *trg_json_widget_entry_new(GList **wl, JsonObject *obj, const gchar *key,
                                     GtkWidget *toggleDep)
{
    GtkWidget *w = gtk_entry_new();
    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);

    wd->saveFunc = trg_json_widget_entry_save;
    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled", G_CALLBACK(toggle_active_arg_is_sensitive),
                         w);
    }

    gtk_entry_set_text(GTK_ENTRY(w), json_object_get_string_member(obj, key));

    *wl = g_list_append(*wl, wd);

    return w;
}

GtkWidget *trg_json_widget_spin_int_new(GList **wl, JsonObject *obj, const gchar *key,
                                        GtkWidget *toggleDep, gint min, gint max, gint step)
{
    GtkWidget *w = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max, (gdouble)step);

    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(w), 0);

    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);

    wd->saveFunc = trg_json_widget_spin_int_save;
    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled", G_CALLBACK(toggle_active_arg_is_sensitive),
                         w);
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), (double)json_object_get_int_member(obj, key));

    *wl = g_list_append(*wl, wd);

    return w;
}

GtkWidget *trg_json_widget_spin_double_new(GList **wl, JsonObject *obj, const gchar *key,
                                           GtkWidget *toggleDep, gdouble min, gdouble max,
                                           gdouble step)
{
    GtkWidget *w = gtk_spin_button_new_with_range(min, max, step);

    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);

    wd->saveFunc = trg_json_widget_spin_double_save;
    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled", G_CALLBACK(toggle_active_arg_is_sensitive),
                         w);
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), json_object_get_double_member(obj, key));

    *wl = g_list_append(*wl, wd);

    return w;
}

void trg_json_widget_check_save(GtkWidget *widget, JsonObject *obj, gchar *key)
{
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    json_object_set_boolean_member(obj, key, active);
}

void trg_json_widget_entry_save(GtkWidget *widget, JsonObject *obj, gchar *key)
{
    json_object_set_string_member(obj, key, gtk_entry_get_text(GTK_ENTRY(widget)));
}

void trg_json_widget_spin_int_save(GtkWidget *widget, JsonObject *obj, gchar *key)
{
    json_object_set_int_member(obj, key, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
}

void trg_json_widget_spin_double_save(GtkWidget *widget, JsonObject *obj, gchar *key)
{
    json_object_set_double_member(obj, key, gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
}
