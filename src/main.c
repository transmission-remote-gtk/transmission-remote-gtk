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

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-client.h"
#include "trg-gtk-app.h"
#include "trg-main-window.h"

/* Handle arguments and start the main window. */

/* Get platform dependendent localedir */
static void bindtext_wrapper(void)
{
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#ifdef G_OS_WIN32
    gchar *moddir = g_win32_get_package_installation_directory_of_module(NULL);

    gchar *localedir = g_build_path(G_DIR_SEPARATOR_S, moddir, "share", "locale", NULL);

    bindtextdomain(GETTEXT_PACKAGE, localedir);
    g_free(moddir);
    g_free(localedir);
#else
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
#endif
}

int main(int argc, char *argv[])
{
    g_set_application_name(PACKAGE_NAME);
    bindtext_wrapper();

    g_autoptr(TrgClient) client = trg_client_new();
    g_autoptr(TrgGtkApp) gtk_app = trg_gtk_app_new(client);
    return g_application_run(G_APPLICATION(gtk_app), argc, argv);
}
