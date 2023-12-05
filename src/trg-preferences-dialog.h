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
#include <glib.h>
#include <gtk/gtk.h>

#include "trg-main-window.h"

#define TRG_TYPE_PREFERENCES_DIALOG (trg_preferences_dialog_get_type())
G_DECLARE_FINAL_TYPE(TrgPreferencesDialog, trg_preferences_dialog, TRG, PREFERENCES_DIALOG,
                     GtkDialog)

typedef struct {
    GtkWidget *widget;
    int flags;
    gchar *key;
    void (*saveFunc)(TrgPrefs *, void *);
    void (*refreshFunc)(TrgPrefs *, void *);
} trg_pref_widget_desc;

GtkWidget *trg_preferences_dialog_get_instance(TrgMainWindow *win, TrgClient *client);
trg_pref_widget_desc *trg_pref_widget_desc_new(GtkWidget *w, gchar *key, int flags);
void trg_preferences_dialog_set_page(TrgPreferencesDialog *pref_dlg, guint page);
