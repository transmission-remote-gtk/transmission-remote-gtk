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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <fontconfig/fontconfig.h>

#include "trg-gtk-app.h"
#if WIN32
#include "win32-mailslot.h"
#endif

#include "trg-main-window.h"
#include "trg-client.h"
#include "util.h"

/* Handle arguments and start the main window.
 *
 * either GtkApplication - replaces libunique, GTK3 only, and non-win32.
 * or win32 API mailslots.
 *
 * win32 could possibly run from GtkApplication now, mailslots were needed
 * for GTK2 (support removed).
 */

#if !WIN32

/* GtkApplication - the replacement for libunique.
 * This is implemented in trg-gtk-app.c
 */

static gint trg_gtkapp_init(TrgClient * client, int argc, char *argv[])
{
    TrgGtkApp *gtk_app = trg_gtk_app_new(client);

    gint exitCode = g_application_run(G_APPLICATION(gtk_app), argc, argv);

    g_object_unref(gtk_app);

    return exitCode;
}

#elif WIN32

static gint
trg_win32_init(TrgClient * client, int argc, char *argv[], gchar ** args)
{
    gchar *moddir =
        g_win32_get_package_installation_directory_of_module(NULL);
    gchar *localedir =
        g_build_path(G_DIR_SEPARATOR_S, moddir, "share", "locale",
                     NULL);

    bindtextdomain(GETTEXT_PACKAGE, localedir);
    g_free(moddir);
    g_free(localedir);

    if (!mailslot_send_message(args)) {
        TrgMainWindow *window =
            trg_main_window_new(client, should_be_minimised(argc, argv));
        trg_main_window_set_start_args(window, args);
        auto_connect_if_required(window);
        mailslot_start_background_listener(window);
        gtk_main();
    }

    return EXIT_SUCCESS;
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

/* Win32 mailslots. I've implemented this in win32-mailslot.c */

#if !WIN32
static void trg_non_win32_init(void)
{
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
}
#endif

static void trg_cleanup(void)
{
    curl_global_cleanup();
}

#if WIN32

static gchar **convert_args(int argc, char *argv[])
{
    gchar *cwd = g_get_current_dir();
    gchar **files = NULL;

    if (argc > 1) {
        GSList *list = NULL;
        int i;

        for (i = 1; i < argc; i++) {
            if (is_minimised_arg(argv[i])) {
                continue;
            } else if (!is_url(argv[i]) && !is_magnet(argv[i])
                       && g_file_test(argv[i], G_FILE_TEST_IS_REGULAR)
                       && !g_path_is_absolute(argv[i])) {
                list = g_slist_append(list,
                                      g_build_path(G_DIR_SEPARATOR_S, cwd,
                                                   argv[i], NULL));
            } else {
                list = g_slist_append(list, g_strdup(argv[i]));
            }
        }

        if (list) {
            GSList *li;
            files = g_new0(gchar *, g_slist_length(list) + 1);
            i = 0;
            for (li = list; li; li = g_slist_next(li)) {
                files[i++] = li->data;
            }
            g_slist_free(list);
        }
    }

    g_free(cwd);

    return files;
}

#endif

int main(int argc, char *argv[])
{
#if WIN32
    gchar **args;
#endif
    gint exitCode = EXIT_SUCCESS;
    TrgClient *client;

    gtk_init(&argc, &argv);

#if WIN32
    args = convert_args(argc, argv);
#endif

    curl_global_init(CURL_GLOBAL_ALL);
    client = trg_client_new();

    g_set_application_name(PACKAGE_NAME);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

#ifdef WIN32
    exitCode = trg_win32_init(client, argc, argv, args);
#else
    trg_non_win32_init();
    exitCode = trg_gtkapp_init(client, argc, argv);
#endif

    g_object_unref(client);

    trg_cleanup();

    return exitCode;
}
