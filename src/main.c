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

#include <curl/curl.h>
#include <curl/easy.h>

#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <fontconfig/fontconfig.h>

#include "trg-gtk-app.h"

#include "trg-main-window.h"
#include "trg-client.h"

/* Handle arguments and start the main window. */

#ifndef G_OS_WIN32

static gint trg_gtkapp_init(TrgClient * client, int argc, char *argv[])
{
    TrgGtkApp *gtk_app = trg_gtk_app_new(client);

    gint exitCode = g_application_run(G_APPLICATION(gtk_app), argc, argv);

    g_object_unref(gtk_app);

    return exitCode;
}

#elif defined G_OS_WIN32

static gint
trg_win32_init(TrgClient * client, int argc, char *argv[])
{
    gchar *moddir =
        g_win32_get_package_installation_directory_of_module(NULL);
    gchar *localedir =
        g_build_path(G_DIR_SEPARATOR_S, moddir, "share", "locale",
                     NULL);

    bindtextdomain(GETTEXT_PACKAGE, localedir);
    g_free(moddir);
    g_free(localedir);

    TrgGtkApp *gtk_app = trg_gtk_app_new(client);
    gint exitCode = g_application_run(G_APPLICATION(gtk_app), argc, argv);
    return exitCode;
}

#else

static gint
trg_simple_init(TrgClient * client, int argc, char *argv[], gchar ** args)
{
    TrgMainWindow *window =
        trg_main_window_new(client, should_be_minimised(argc, argv));
    trg_main_window_set_start_args(window, args);
    auto_connect_if_required(window);
    gtk_main();

    return EXIT_SUCCESS;
}

#endif

#ifndef G_OS_WIN32
static void trg_non_win32_init(void)
{
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
}
#endif

static void trg_cleanup(void)
{
    curl_global_cleanup();
}

int main(int argc, char *argv[])
{
    gint exitCode = EXIT_SUCCESS;
    TrgClient *client;

    gtk_init(&argc, &argv);

    curl_global_init(CURL_GLOBAL_ALL);
    client = trg_client_new();

    g_set_application_name(PACKAGE_NAME);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

#ifdef G_OS_WIN32
    exitCode = trg_win32_init(client, argc, argv);
#else
    trg_non_win32_init();
    exitCode = trg_gtkapp_init(client, argc, argv);
#endif

    g_object_unref(client);

    trg_cleanup();

    return exitCode;
}
