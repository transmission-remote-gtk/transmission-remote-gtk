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

#include <gtk/gtk.h>

#include "icon-turtle.h"

typedef struct {
    const guint8 *raw;
    const char *name;
} BuiltinIconInfo;

static const BuiltinIconInfo my_fallback_icons[] = {
    {blue_turtle, "alt-speed-on"},
    {grey_turtle, "alt-speed-off"}
};

void register_my_icons(GtkIconTheme * theme)
{
    int i;
    const int n = G_N_ELEMENTS(my_fallback_icons);
    GtkIconFactory *factory = gtk_icon_factory_new();

    gtk_icon_factory_add_default(factory);

    for (i = 0; i < n; ++i) {
        const char *name = my_fallback_icons[i].name;

        if (!gtk_icon_theme_has_icon(theme, name)) {
            int width;
            GdkPixbuf *p;
            GtkIconSet *icon_set;

            p = gdk_pixbuf_new_from_inline(-1, my_fallback_icons[i].raw,
                                           FALSE, NULL);
            width = gdk_pixbuf_get_width(p);
            icon_set = gtk_icon_set_new_from_pixbuf(p);
            gtk_icon_theme_add_builtin_icon(name, width, p);
            gtk_icon_factory_add(factory, name, icon_set);

            g_object_unref(p);
            gtk_icon_set_unref(icon_set);
        }
    }

    g_object_unref(G_OBJECT(factory));
}
