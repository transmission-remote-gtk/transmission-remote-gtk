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

#if !GTK_CHECK_VERSION( 3, 0, 0 ) && HAVE_LIBUNIQUE
#include <unique/unique.h>
#elif GTK_CHECK_VERSION( 3, 0, 0 )
#include "trg-gtk-app.h"
#elif WIN32
#include "win32-mailslot.h"
#endif

#include "trg-main-window.h"
#include "trg-client.h"
#include "util.h"

/* Handle arguments and start the main window. Unfortunately, there's three
 * different ways to achieve a unique instance and pass arguments around. :(
 *
 * 1) libunique - GTK2 (non-win32). deprecated in GTK3 for GtkApplication.
 * 2) GtkApplication - replaces libunique, GTK3 only, and non-win32.
 * 3) win32 API mailslots.
 */

/*
 * libunique.
 */

#if !GTK_CHECK_VERSION( 3, 0, 0 ) && HAVE_LIBUNIQUE

enum {
    COMMAND_0,
    COMMAND_ADD
};

static UniqueResponse
message_received_cb(UniqueApp * app G_GNUC_UNUSED,
                    gint command,
                    UniqueMessageData * message,
                    guint time_, gpointer user_data)
{
    TrgMainWindow *win;
    UniqueResponse res;
    gchar **uris;

    win = TRG_MAIN_WINDOW(user_data);

    switch (command) {
    case UNIQUE_ACTIVATE:
        gtk_window_set_screen(GTK_WINDOW(user_data),
                              unique_message_data_get_screen(message));
        gtk_window_present_with_time(GTK_WINDOW(user_data), time_);
        res = UNIQUE_RESPONSE_OK;
        break;
    case COMMAND_ADD:
        uris = unique_message_data_get_uris(message);
        res =
            trg_add_from_filename(win,
                                  uris) ? UNIQUE_RESPONSE_OK :
            UNIQUE_RESPONSE_FAIL;
        break;
    default:
        res = UNIQUE_RESPONSE_OK;
        break;
    }

    return res;
}

static gint
trg_libunique_init(TrgClient * client, int argc,
                   gchar * argv[], gchar ** args)
{
    UniqueApp *app = unique_app_new_with_commands("uk.org.eth0.trg", NULL,
                                                  "add", COMMAND_ADD,
                                                  NULL);
    TrgMainWindow *window;

    if (unique_app_is_running(app)) {
        UniqueCommand command;
        UniqueResponse response;
        UniqueMessageData *message;

        if (args) {
            command = COMMAND_ADD;
            message = unique_message_data_new();
            unique_message_data_set_uris(message, args);
            g_strfreev(args);
        } else {
            command = UNIQUE_ACTIVATE;
            message = NULL;
        }

        response = unique_app_send_message(app, command, message);
        unique_message_data_free(message);

        if (response != UNIQUE_RESPONSE_OK)
            return EXIT_FAILURE;
    } else {
        window =
            trg_main_window_new(client, should_be_minimised(argc, argv));
        g_signal_connect(app, "message-received",
                         G_CALLBACK(message_received_cb), window);

        trg_main_window_set_start_args(window, args);
        auto_connect_if_required(window);
        gtk_main();
    }

    g_object_unref(app);

    return EXIT_SUCCESS;
}

#elif !WIN32 && GTK_CHECK_VERSION( 3, 0, 0 )

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
static void trg_non_win32_init()
{
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
}
#endif

static void trg_cleanup()
{
    curl_global_cleanup();
}

#if WIN32 || !GTK_CHECK_VERSION( 3, 0, 0 )

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
#if WIN32 || !GTK_CHECK_VERSION( 3, 0, 0 )
    gchar **args;
#endif
    gint exitCode = EXIT_SUCCESS;
    TrgClient *client;

    g_type_init();
    g_thread_init(NULL);
    gtk_init(&argc, &argv);

#if WIN32 || !GTK_CHECK_VERSION( 3, 0, 0 )
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
#if !GTK_CHECK_VERSION( 3, 0, 0 ) && HAVE_LIBUNIQUE
    exitCode = trg_libunique_init(client, argc, argv, args);
#elif GTK_CHECK_VERSION( 3, 0, 0 )
    exitCode = trg_gtkapp_init(client, argc, argv);
#else
    exitCode = trg_simple_init(client, argc, argv, args);
#endif
#endif

    trg_cleanup();

    return exitCode;
}
