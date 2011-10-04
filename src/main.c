/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
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
#ifdef HAVE_LIBUNIQUE
#include <unique/unique.h>
#endif

#include "trg-main-window.h"
#include "trg-client.h"
#include "util.h"

#ifdef HAVE_LIBUNIQUE

#define TRG_LIBUNIQUE_DOMAIN "uk.org.eth0.trg"

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
#endif

static gboolean should_be_minimised(int argc, char *argv[])
{
    int i;
    for(i = 1; i < argc; i++)
        if (!g_strcmp0(argv[i], "-m") || !g_strcmp0(argv[i], "--minimized"))
            return TRUE;

    return FALSE;
}

static gchar **convert_args(int argc, char *argv[])
{
    gchar *cwd = g_get_current_dir ();
    gchar **files = NULL;
    int i;
    if (argc > 1) {
        files = g_new0(gchar *, argc);
        for (i = 1; i < argc; i++) {
            if (!is_url(argv[i]) && !is_magnet(argv[i])
                    && g_file_test(argv[i], G_FILE_TEST_IS_REGULAR)
                    && !g_path_is_absolute(argv[i])) {
                files[i - 1] = g_build_path(G_DIR_SEPARATOR_S, cwd, argv[i], NULL);
            } else {
                files[i - 1] = g_strdup(argv[i]);
            }
        }
    }

    g_free(cwd);

    return files;
}

int main(int argc, char *argv[])
{
    int returnValue = EXIT_SUCCESS;
    TrgMainWindow *window;
    TrgClient *client;
    gchar **args = convert_args(argc, argv);
#ifdef HAVE_LIBUNIQUE
    UniqueApp *app = NULL;
    gboolean withUnique;
#endif

#ifdef TRG_MEMPROFILE
    GMemVTable gmvt = {malloc,realloc,free,calloc,malloc,realloc};
    g_mem_set_vtable(&gmvt);
    g_mem_set_vtable(glib_mem_profiler_table);
    g_mem_profile();
#endif
    
    g_type_init();
    g_thread_init(NULL);
    gtk_init(&argc, &argv);

    g_set_application_name (PACKAGE_NAME);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

#ifdef HAVE_LIBUNIQUE
    if ((withUnique = g_getenv("TRG_NOUNIQUE") == NULL))
        app = unique_app_new_with_commands(TRG_LIBUNIQUE_DOMAIN, NULL,
                                           "add", COMMAND_ADD, NULL);

    if (withUnique && unique_app_is_running(app)) {
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
            returnValue = EXIT_FAILURE;
    } else {
#endif
        client = trg_client_new();

        curl_global_init(CURL_GLOBAL_ALL);

        window = trg_main_window_new(client, should_be_minimised(argc, argv));

#ifdef HAVE_LIBUNIQUE
        if (withUnique) {
            unique_app_watch_window(app, GTK_WINDOW(window));
            g_signal_connect(app, "message-received",
                             G_CALLBACK(message_received_cb), window);
        }
#endif

        auto_connect_if_required(window, args);
        gtk_main();

        curl_global_cleanup();
#ifdef HAVE_LIBUNIQUE
    }

    if (withUnique)
        g_object_unref(app);
#endif

    return returnValue;
}
