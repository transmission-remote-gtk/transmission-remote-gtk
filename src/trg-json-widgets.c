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
#include "util.h"

void trg_json_widgets_save(GList *list, JsonObject *out)
{
    GList *li;
    for (li = list; li; li = g_list_next(li))
    {
        trg_json_widget_desc *wd = (trg_json_widget_desc*)li->data;
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
    GList *li;
    for (li = list; li; li = g_list_next(li))
        trg_json_widget_desc_free((trg_json_widget_desc*)li->data);

    g_list_free(list);
}

void toggle_active_arg_is_sensitive(GtkToggleButton * b, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data),
                             gtk_toggle_button_get_active(b));
}

GtkWidget *trg_json_widget_check_new(GList **wl, JsonObject *obj, const gchar *key, const gchar *label, GtkWidget *toggleDep)
{
    GtkWidget *w = gtk_check_button_new_with_mnemonic(label);
    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);

    wd->saveFunc = trg_json_widget_check_save;
    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled",
                G_CALLBACK(toggle_active_arg_is_sensitive), w);
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), json_object_get_boolean_member(obj, key));

    *wl = g_list_append(*wl, wd);

    return w;
}

GtkWidget *trg_json_widget_entry_new(GList **wl, JsonObject *obj, const gchar *key, GtkWidget *toggleDep)
{
    GtkWidget *w = gtk_entry_new();
    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);

    wd->saveFunc = trg_json_widget_entry_save;
    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled",
                G_CALLBACK(toggle_active_arg_is_sensitive), w);
    }

    gtk_entry_set_text(GTK_ENTRY(w), json_object_get_string_member(obj, key));

    *wl = g_list_append(*wl, wd);

    return w;
}

static GtkWidget *trg_json_widget_spin_common_new(GList **wl, JsonObject *obj,
        const gchar *key, GtkWidget *toggleDep, trg_json_widget_spin_type type, gint min,
        gint max, gdouble step)
{
    GtkWidget *w = gtk_spin_button_new_with_range(min, max, step);
    trg_json_widget_desc *wd = g_new0(trg_json_widget_desc, 1);
    JsonNode *node = json_object_get_member(obj, key);

    if (type == TRG_JSON_WIDGET_SPIN_DOUBLE)
        wd->saveFunc = trg_json_widget_spin_save_double;
    else
        wd->saveFunc = trg_json_widget_spin_save_int;

    wd->key = g_strdup(key);
    wd->widget = w;

    if (toggleDep) {
        gtk_widget_set_sensitive(w, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleDep)));
        g_signal_connect(G_OBJECT(toggleDep), "toggled",
                G_CALLBACK(toggle_active_arg_is_sensitive), w);
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), json_node_really_get_double(node));

    *wl = g_list_append(*wl, wd);

    return w;
}

GtkWidget *trg_json_widget_spin_new_int(GList **wl, JsonObject *obj, const gchar *key, GtkWidget *toggleDep,
        gint min, gint max, gint step)
{
    return trg_json_widget_spin_common_new(wl, obj, key, toggleDep, TRG_JSON_WIDGET_SPIN_INT, min, max, (gdouble)step);
}

GtkWidget *trg_json_widget_spin_new_double(GList **wl, JsonObject *obj, const gchar *key, GtkWidget *toggleDep,
        gint min, gint max, gdouble step)
{
    return trg_json_widget_spin_common_new(wl, obj, key, toggleDep, TRG_JSON_WIDGET_SPIN_DOUBLE, min, max, step);
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

void trg_json_widget_spin_save_int(GtkWidget *widget, JsonObject *obj, gchar *key)
{
    json_object_set_int_member(obj, key, (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
}

void trg_json_widget_spin_save_double(GtkWidget *widget, JsonObject *obj, gchar *key)
{
    json_object_set_double_member(obj, key, gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
}
